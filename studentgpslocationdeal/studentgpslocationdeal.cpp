

#include "studentgpslocationdeal.h"
#include "Configure.h"

DBAccess g_dbaccess;

CConfigure g_conf;
zlog_category_t* g_server_cat = NULL;




int GetModuleFileName(char* name, int size)
{
    FILE* stream = fopen("/proc/self/cmdline", "r");
    fgets(name, size, stream);
    fclose(stream);

    return strlen(name);
}

amqp_connection_state_t ConnectRmq(amqp_bytes_t consumerexchange,char* routingkey)
{
    const char *hostname = g_conf.GetRmqServerIP();  
    int port = g_conf.GetRmqServerPort(); 
    const char* user = g_conf.GetRmqUser();
    const char* passwd = g_conf.GetRmqPasswd();
    
    amqp_bytes_t queuename;
    queuename.bytes = (char *)g_conf.GetQueueName();
    queuename.len = strlen(g_conf.GetQueueName());

    //char const *programid = g_conf.GetProgramID();
    sprintf(routingkey,"#.%x.#",DATA_STU_GPS_LOCATION);
    amqp_bytes_t amqpkey;
    amqpkey.bytes = routingkey;
    amqpkey.len = strlen(routingkey);
  
    amqp_socket_t *socket = NULL;  
    int status = 1; 

    amqp_connection_state_t conn = amqp_new_connection();
    socket = amqp_tcp_socket_new(conn);  
    if (!socket) 
    {    
       die("creating TCP socket");  
    }
    status = amqp_socket_open(socket, hostname, port);  
    if (status) 
    {    
        die("opening TCP socket");  
    }
    die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,user, passwd),"Logging in");
    amqp_channel_open(conn, 1);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");
    amqp_exchange_declare(conn,1,consumerexchange,amqp_cstring_bytes("topic"),0,1,0,0,amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring exchange");
    
    /*amqp_exchange_declare(conn,1,respexchange,amqp_cstring_bytes("topic"),0,1,0,0,amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring exchange");*/

    amqp_queue_declare(conn, 1, queuename, 0, 1, 0, 0,amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");

    //获取routingkey
  
    amqp_queue_bind(conn, 1, queuename,consumerexchange, amqpkey,amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");

    amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 0, 0, amqp_empty_table);

    die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

    return conn;
   
}

void CloseRmqConection(amqp_connection_state_t conn)
{
    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");  
    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");  
    die_on_error(amqp_destroy_connection(conn), "Ending connection");
}


/************************************************************************/
/*CoreThread*/
/************************************************************************/
void *CoreThread(void *args)
{
    pthread_detach (pthread_self());
    
    amqp_bytes_t consumerexchange;
    consumerexchange.bytes = (char *)g_conf.GetConSumerExchange();
    consumerexchange.len = strlen(g_conf.GetConSumerExchange());

    /*amqp_bytes_t respexchange;
    respexchange.bytes = (char *)g_conf.GetRespExchange();
    respexchange.len = strlen(g_conf.GetRespExchange());*/

    char  conroutingkey[ROUTINGKEY_LEN]={0};
    
    amqp_connection_state_t conn;

    conn = ConnectRmq(consumerexchange,conroutingkey);

    
	while (1) 
    {

        amqp_rpc_reply_t res;      
        amqp_envelope_t envelope;      
        amqp_maybe_release_buffers(conn);
        int cousumeintervaltime = g_conf.GetConsumeIntervalTime();
        struct  timeval tv;
        tv.tv_sec = 0; 
        tv.tv_usec = cousumeintervaltime * 1000; 

        res = amqp_consume_message(conn, &envelope,&tv,0);
        
        if (AMQP_RESPONSE_NORMAL != res.reply_type) 
        {  

            
           if(AMQP_RESPONSE_LIBRARY_EXCEPTION == res.reply_type &&
              AMQP_STATUS_TIMEOUT == res.library_error)
           {
                continue;
           }
           else
           {    
               zlog_error(g_server_cat,"res.reply_type is %d,"
                     "res.library_error is %d !",res.reply_type,res.library_error);  
                CloseRmqConection(conn);
                conn = ConnectRmq(consumerexchange,conroutingkey);
                sleep(1);
           }
           continue;      
        }

        unsigned int iconsumerlen = envelope.message.body.len;
        char *prmqconsumer = new char[iconsumerlen+1];
        memset(prmqconsumer,0x0,iconsumerlen+1);
        memcpy(prmqconsumer,envelope.message.body.bytes,iconsumerlen);

        int iresproutingkeylen = envelope.routing_key.len;

        zlog_info(g_server_cat, "consume a amqpmsg from rabbitmq server,"
            "routingkey is %s,routingkey size is %d,message size is %d",
            (char *)envelope.routing_key.bytes,iresproutingkeylen,iconsumerlen);
        hzlog_debug(g_server_cat, prmqconsumer, iconsumerlen);


        bool bresult = g_dbaccess.GpsInsertDB(prmqconsumer);

         
        if(TRUE == bresult)
        {
            amqp_basic_ack(conn,1,envelope.delivery_tag,0);
            amqp_destroy_envelope(&envelope);            
        }
        else
        {
            amqp_basic_reject(conn,1,envelope.delivery_tag,1);
            amqp_destroy_envelope(&envelope);
            sleep(1);
        }
        delete []prmqconsumer;
        
    }

    CloseRmqConection(conn);
    pthread_exit(NULL);
}



int main (int argc, char *argv[], char *envp[])
{
    //check if need to display the version of this patch
    
    char szFileName[260] = {0};
    GetModuleFileName(szFileName, 260);

    writePid(PROCESSNAME);

    
    int nIndex = strlen(szFileName);
	while (nIndex > 0 && szFileName[nIndex - 1] != '/') 
		{nIndex--;}

	szFileName[nIndex] = 0;

    strncat(szFileName, "studentgpslocationdeal_log.conf", 260);
    int ret = zlog_init(szFileName);	
  	if (ret != 0) 
	{
    	printf("Initlize zlog faield\n");
    	exit(EXIT_FAILURE);
  	}

    g_server_cat = zlog_get_category("server_cat");
    if (!g_server_cat) 
	{
    	printf("zlog_get_category failed\n");
    	exit(EXIT_FAILURE);
  	}

    nIndex = strlen(szFileName);
	while (nIndex > 0 && szFileName[nIndex - 1] != '/')
		{nIndex--;}

	szFileName[nIndex] = 0;
	strncat(szFileName, "aoptions.xml", 260);
    
    bool bResult = g_conf.LoadOptions(szFileName);
    char szLogFileName[260] = {0};
    GetModuleFileName(szLogFileName, 260);

    if (false == bResult)
    {
        zlog_error(g_server_cat, "There is something wrong about the configure file aoptions.xml"
            " please do a confirm");
        exit(EXIT_FAILURE);
    }
    
    g_dbaccess.Init(g_conf.m_dbconf);
	
	while(1)
	{
		zlog_info(g_server_cat,"dbconnNum = %d",g_conf.m_dbconf.dbconnNum);
		
		int dbnum = g_dbaccess.connDB();
		if (dbnum != g_conf.m_dbconf.dbconnNum)
		{
			zlog_info(g_server_cat,"connect db error");
			g_dbaccess.disconn();
			sleep(1);
			continue;
		}
		break;
	}
 
    zlog_info(g_server_cat,"Main Thread Startup!");

    pthread_t hListener = NULL;

    int nmaxthread_nbr = g_conf.m_nChildThreadPoolCount;
    for (int nthread_nbr = 0; nthread_nbr < nmaxthread_nbr; nthread_nbr++)
	{
	    int iError = pthread_create (&hListener,NULL,CoreThread,NULL);
        if (iError != 0)
        {
            zlog_info(g_server_cat,"Create CoreThread Thread Fail !\n");
            return -4;
        }
        usleep(10);
	}
    while (1)
	{
		
		sleep(1);
		
	}
    
    return 0;
}
