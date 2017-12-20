
#include "GPSDataRecvier.h"
#include "RecogizerListener.h"
#include "RecogizerSocket.h"
#include "Configure.h"
#include "Tool.h"
#include "DataDef.h"					
#include "zlog.h"
#include<list>
#include "zmq.hpp"
#include<string>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include "utils.h"
#include <stdlib.h>
#define MAX_PATH 1024

bool		g_isExit1;							
bool		g_isExit2;							
bool		g_isExit3;							
bool		g_isExit4;				
CConfigure	g_configure;		

zlog_category_t* g_server_cat = NULL;
DBAccess g_dbaccess;


#ifndef linux
TCHAR		g_szFilePath[MAX_PATH] = {0};
TCHAR		g_szFilePath1[MAX_PATH] = {0};		

HANDLE		g_hSwitch;						
#else
char		g_szFilePath[MAX_PATH] = {0};		
char		g_szFilePath1[MAX_PATH] = {0};		

sem_t		g_SemSwitch;					
#endif

#ifdef linux
const long double MinProb = (int64_t)1.0 / (int64_t)(RAND_MAX + (int64_t)1);


bool happened(long double probability)//probability 0~1
{
    if(probability <= 0)
    {
        return false;
    }
    if(probability < MinProb)
    {
        return rand() == 0 && happened(probability * (RAND_MAX + (int64_t)1));
    }
    if(rand() <= probability * (RAND_MAX + (int64_t)1))
    {
        return true;
    }
    return false;
}

int myrandom(int n)
{
    int t = 0;
    if(n <= RAND_MAX)
    {
        int64_t r = RAND_MAX - (RAND_MAX + (int64_t)1) % n;
        t = rand();
        while ( t > r )
        {
            t = rand();
        }
        return t % n;
    }
    else
    {
        int64_t r = n % (RAND_MAX + (int64_t)1);
        if( happened( (double)r/n ) )
        {
            return n - r + myrandom(r);
        }
        else
        {
            return rand() + myrandom(n / (RAND_MAX + (int64_t)1))*(RAND_MAX + (int64_t)1);
        }
    }
}
#endif
// ****END****  RTMsg User  2013-09-22 14:10:06 add by lichuang
amqp_connection_state_t ConnectRmq(amqp_bytes_t pubexchange)
{
	const char *hostname = g_configure.getrmqhostname().c_str();  
	int port = g_configure.getrmqport(); 
	const char* user = g_configure.GetRmqUser();
	const char* passwd = g_configure.GetRmqPasswd();
	
	
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
	amqp_exchange_declare(conn,1,pubexchange,amqp_cstring_bytes("topic"),0,1,0,0,amqp_empty_table);
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring exchange");
	


	return conn;
	
}

void CloseRmqConection(amqp_connection_state_t conn)
{
	die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");  
	die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");  
	die_on_error(amqp_destroy_connection(conn), "Ending connection");
}

void *MsgDealThread (void *pContext)
{
	pthread_detach (pthread_self());
	amqp_bytes_t pubexchange;
    pubexchange.bytes = (char *)g_configure.getrmqexchanage().c_str();
	pubexchange.len = g_configure.getrmqexchanage().size();
	
	char  routingkey[64]={0};
	char  pchbuffer[1024]={0};
	
	amqp_connection_state_t conn;

	conn = ConnectRmq(pubexchange);
	
	long lSkop = 0;

	void* sksndReqMore=zmq_socket(pContext,ZMQ_PUB);
	zmq_setsockopt(sksndReqMore,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_bind(sksndReqMore,"inproc://PubReqSend");

	void* sksndRspMore=zmq_socket(pContext,ZMQ_PUB);
	zmq_setsockopt(sksndRspMore,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_bind(sksndRspMore,"inproc://PubRspSend");

	void* sksndReq=zmq_socket(pContext,ZMQ_PULL);
	zmq_setsockopt(sksndReq,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_bind(sksndReq,"inproc://ReqSnd");

	void* sksndRsp=zmq_socket(pContext,ZMQ_PULL);
	zmq_setsockopt(sksndRsp,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_bind(sksndRsp,"inproc://RspSnd");

	void* skGpsData=zmq_socket(pContext,ZMQ_PULL);
	zmq_setsockopt(skGpsData,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_bind(skGpsData,"inproc://GpsSnd");


	zmq_pollitem_t pollitems[] = {
		{sksndReq, 0, ZMQ_POLLIN, ZMQ_POLLOUT},
		{sksndRsp, 0, ZMQ_POLLIN, ZMQ_POLLOUT},
        {skGpsData, 0, ZMQ_POLLIN, ZMQ_POLLOUT}
      };

	sem_post(&g_SemSwitch);
	PacketHead head;
	while (1) 
    {
		zmq_poll (pollitems, 3, 1000);
		if (pollitems [0].revents & ZMQ_POLLIN)
		{
			zlog_info(g_server_cat,"recv req .......");
			zmq_msg_t msg;
			int rc = zmq_msg_init (&msg);
			rc = zmq_recvmsg (sksndReq, &msg, 0);
			
		
			cJSON* root = cJSON_Parse((char *)zmq_msg_data(&msg));
			if(cJSON_GetObjectItem(root,"imei") !=NULL && cJSON_GetObjectItem(root,"cmd") !=NULL)
			{
				zlog_info(g_server_cat,"pub req to dev .......");
				char imei[50] = {0};
				sprintf(imei,"%s",cJSON_GetObjectItem(root,"imei")->valuestring);
				char cmd[50] = {0};
				sprintf(cmd,"%s",cJSON_GetObjectItem(root,"cmd")->valuestring);
				
				zmq_msg_t msg;
				zmq_msg_t msg1;
				int rc = zmq_msg_init_size (&msg1, strlen(cmd));
				rc = zmq_msg_init_size (&msg, strlen(imei));
				memcpy((char *)zmq_msg_data(&msg1),cmd,strlen(cmd));
				memcpy((char *)zmq_msg_data(&msg),imei,strlen(imei));
				zlog_info(g_server_cat,"pub req to dev ,imei=%s.......",(char *)zmq_msg_data(&msg));
				rc = zmq_sendmsg(sksndReqMore,&msg,ZMQ_SNDMORE);
				rc = zmq_sendmsg(sksndReqMore,&msg1,ZMQ_NOBLOCK);
			}
		}

		if (pollitems [1].revents & ZMQ_POLLIN)
		{
			zlog_info(g_server_cat,"recv rsp .......");
			zmq_msg_t msg;
			int rc = zmq_msg_init (&msg);
			rc = zmq_recvmsg (sksndRsp, &msg, 0);
			
			cJSON* root = cJSON_Parse((char *)zmq_msg_data(&msg));
			if(cJSON_GetObjectItem(root,"imei") !=NULL && cJSON_GetObjectItem(root,"cmdrsp") !=NULL)
			{
				zlog_info(g_server_cat,"pub rsp to web .......");
				char imei[50] = {0};
				sprintf(imei,"%s",cJSON_GetObjectItem(root,"imei")->valuestring);

				zmq_msg_t msg1;
				int rc = zmq_msg_init_size (&msg1, strlen(imei));
				memcpy((char *)zmq_msg_data(&msg1),imei,strlen(imei));
				zlog_info(g_server_cat,"ZMQ_SNDMORE rsp msg = %s,size = %d",(char *)zmq_msg_data(&msg1),strlen(imei));
				rc = zmq_sendmsg(sksndRspMore,&msg1,ZMQ_SNDMORE);
				rc = zmq_sendmsg(sksndRspMore,&msg,ZMQ_NOBLOCK);
			}
		}  
		
		if (pollitems [2].revents & ZMQ_POLLIN)
		{
			zlog_info(g_server_cat,"recv rsp .......");
			zmq_msg_t msg;
			int rc = zmq_msg_init (&msg);
			rc = zmq_recvmsg (skGpsData, &msg, 0);

			PacketHead* pheader = reinterpret_cast<PacketHead *>(zmq_msg_data(&msg));


			amqp_basic_properties_t props;    
            props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;    
            props.content_type = amqp_cstring_bytes("text/plain");
            props.delivery_mode = 2;

            sprintf(routingkey,"%x.%x.%s",pheader->wDevType,pheader->wMsgType,pheader->szImei);
            amqp_bytes_t amqpkey;
            amqpkey.bytes = routingkey;
            amqpkey.len = strlen(routingkey);
            
            zlog_info(g_server_cat,"publish a msg,routingkey is %s",routingkey);

            amqp_bytes_t amqpmsg;
            amqpmsg.bytes = (char *)zmq_msg_data(&msg)+sizeof(PacketHead);
            amqpmsg.len = zmq_msg_size(&msg)-sizeof(PacketHead);

            zlog_info(g_server_cat, "publish a amqpmsg to rabbitmq server.");
            hzlog_debug(g_server_cat, amqpmsg.bytes, amqpmsg.len);
            
            int ret= amqp_basic_publish(conn,1,pubexchange,amqpkey,
					 0,0,&props,amqpmsg);
			if (0 != ret)
			{
				CloseRmqConection(conn);
				conn = ConnectRmq(pubexchange);
				amqp_basic_publish(conn,1,pubexchange,amqpkey,
					0,0,&props,amqpmsg);
			}
		}    
   }
	 
   zlog_info(g_server_cat,"msg deal thread exit .......");
	pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
	if(argc >=2)
	{
		if(strcmp("-v", argv[1]) == 0 || strcmp("-V", argv[1]) == 0)
		{
			printf("%s\n", STR_APPVER);
			return 0;
		}
	}


	g_isExit1 = false;				
	g_isExit2 = false;						
	g_isExit3 = false;					
	g_isExit4 = false;					


	signal(SIGTERM,handleSignal);

	long lSkop = 0;
	void *ctx_=zmq_init(1);
	void* z_socket=zmq_socket(ctx_,ZMQ_PUSH);
	zmq_setsockopt(z_socket,ZMQ_LINGER,&lSkop,sizeof(lSkop));
		
	
	HANDLE hLoger = NULL;	
	HANDLE hSender = NULL;		
	HANDLE hListener = NULL;		
	

	pid_t org_pid = exist_instance(STR_APPNAME);
	if (org_pid)
	{
		if(argc == 2 && strstr(argv[1],"Exit") != NULL)
		{
			printf("%s suicideing ! \n",STR_APPNAME);
			sendExitSignal(org_pid);
			return 0;
		}
		else
		{
			printf("ERROR:Only one instance can be exists ! \n");
			return 1;
		}
	}

	if(argc == 2 && strstr(argv[1],"Exit") != NULL)
		return 0;

	if(!writePid(STR_APPNAME))
	{
		printf("ERROR:Wirte pid file fail ! \n");
		return 1;
	}

	Assistant::GetCurPath(g_szFilePath,sizeof(g_szFilePath));
	rindex(g_szFilePath,'/')[1] = 0;
	strcat(g_szFilePath,"base_config.xml");

	

  if(g_configure.LoadBasicConfig(g_szFilePath) ==false)
  	{
       printf("There is something wrong about the configure file base_config.xml"
            "  please do a confirm\n");
       exit(1);     
    }

	rindex(g_szFilePath,'/')[1] = 0;
	strcat(g_szFilePath,"GPSDataRecvier_log.conf");
	int ret = zlog_init(g_szFilePath);	
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

	rindex(g_szFilePath,'/')[1] = 0;		

	sem_init(&g_SemSwitch, 0, 0);
/*
	g_dbaccess.Init(g_configure.m_dbconf);
	
	while(1)
	{
		zlog_info(g_server_cat,"dbconnNum = %d",g_configure.m_dbconf.dbconnNum);
		
		int dbnum = g_dbaccess.connDB();
		if (dbnum != g_configure.m_dbconf.dbconnNum)
		{
			zlog_info(g_server_cat,"connect db error");
			g_dbaccess.disconn();
			sleep(1);
			continue;
		}
		break;
	}
	*/
	int iError = 0;

	iError = pthread_create (&hListener,NULL,MsgDealThread,ctx_);
	if (iError != 0)
	{
		zlog_info(g_server_cat,"Create MsgDeal Thread Fail !\n");
		return -4;
	}
	zlog_info(g_server_cat,"Create MsgDeal Thread success!");
	usleep(300*1000);
	sem_wait(&g_SemSwitch);

	iError = pthread_create (&hListener,NULL,ThreadWebListener,ctx_);
	if (iError != 0)
	{
		zlog_info(g_server_cat,"Create web listener Thread Fail !\n");
		return -4;
	}
	zlog_info(g_server_cat,"Create web listener Thread success!");
	usleep(300*1000);
	sem_wait(&g_SemSwitch);

	iError = pthread_create (&hListener,NULL,ThreadListener,ctx_);
	if (iError != 0)
	{
		zlog_info(g_server_cat,"Create Listener Thread Fail !\n");
		return -4;
	}
	zlog_info(g_server_cat,"Create Listener Thread success!");
	usleep(300*1000);
	sem_wait(&g_SemSwitch);
	sem_destroy(&g_SemSwitch);
	
	while(1)
	{
		sleep(1);
	}
	
	while (1)
	{
		sleep(1);
	}
	


	printf("Press AnyKey To Exit!\n");
	//getchar();
	return 0;
}




void * ThreadListener(void *pContext)
{
	pthread_detach (pthread_self());


						



	sem_post(&g_SemSwitch);

	
	RecogizerListener  svrListener;
	svrListener.Initial(g_configure.m_port,pContext,ThreadWorker);		

	while (!g_isExit1)
	{
		if (!svrListener.CreateBinding())		
		{
			zlog_info(g_server_cat,"CreateBinding @Local Port %d Fail !",g_configure.m_port);

			usleep(1000*1000);

		}
		else
		{
			break;
		}
	}


	while (!g_isExit1)		
	{
		int iRet = svrListener.ListenAndAcc();
		if( RET_CLIENT_CONNECTED == iRet)				
		{
			continue;
		}else if ( RET_LISTEN_TIMEOUT == iRet)			
		{
			//svrListener.CheckThread();
			continue;
		}
		else	//
		{
			zlog_info(g_server_cat,"an exception occured when listening !! RecogizerListener Thread exited!");
			svrListener.EndWork();

			pthread_exit(NULL);

		}
	}
	svrListener.EndWork();
	

	pthread_exit(NULL);

}

void * ThreadWebListener(void *pContext)
{
	pthread_detach (pthread_self());


	long lSkop = 0;
	
	zmq::context_t *pZContext = (zmq::context_t *)pContext;	
	//zmq::socket_t skLog(*pZContext,ZMQ_PUSH);			
	//skLog.setsockopt(ZMQ_LINGER,&lSkop,sizeof(lSkop));	
	//skLog.connect("inproc://Logger");						



	sem_post(&g_SemSwitch);

	
	RecogizerListener  svrListener;
	svrListener.Initial(g_configure.m_webPort,pContext,ThreadWebWorker);		

	while (!g_isExit1)
	{
		if (!svrListener.CreateBinding())		
		{
			zlog_info(g_server_cat,"CreateBinding @Local Port %d Fail !",g_configure.m_webPort);

			usleep(1000*1000);

		}
		else
		{
			break;
		}
	}


	while (!g_isExit1)		
	{
		int iRet = svrListener.ListenAndAcc();
		if( RET_CLIENT_CONNECTED == iRet)				
		{
			continue;
		}else if ( RET_LISTEN_TIMEOUT == iRet)			
		{
			//svrListener.CheckThread();
			continue;
		}
		else	//
		{
			zlog_info(g_server_cat,"an exception occured when listening !! RecogizerListener Thread exited!");
			svrListener.EndWork();

			pthread_exit(NULL);

		}
	}
	svrListener.EndWork();
	

	pthread_exit(NULL);

}


void * ThreadWorker(void *pContext)
{
	pthread_detach (pthread_self());
	ThreadContext *pThrContext = (ThreadContext *)pContext;	
	void *pZContext = pThrContext->pZCntext;	
	long lSkop = 0;



	void* sksndMore=zmq_socket(pZContext,ZMQ_SUB);
	zmq_setsockopt(sksndMore,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_connect(sksndMore,"inproc://PubReqSend");

	void* sksndRsp=zmq_socket(pZContext,ZMQ_PUSH);
	zmq_setsockopt(sksndRsp,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_connect(sksndRsp,"inproc://RspSnd");

	void* sksndGps=zmq_socket(pZContext,ZMQ_PUSH);
	zmq_setsockopt(sksndGps,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_connect(sksndGps,"inproc://GpsSnd");


	int iRet = 0;
	RecogizerSocket  RecSocket;
							


	RecSocket.Initial(&g_isExit2,pThrContext,sksndMore,sksndRsp,sksndGps);	
	iRet = RecSocket.AutoConfSocket();
	if(-1 == iRet)		
	{
		pthread_exit(NULL);
	}
	iRet = RecSocket.StartWork();	


	pthread_exit(NULL);
}
int Send2Web(SOCKET clisocket, const void* lpBuf, int nBufLen )
{   
	zlog_info(g_server_cat,"send data to web:");
	hzlog_debug(g_server_cat,lpBuf,nBufLen);
	int iRet = SOCKET_ERROR;
	int ileng = nBufLen;			
	int iPos = 0;					
	while(1)						
	{
		iRet = send(clisocket,(char *)lpBuf+iPos,ileng,0);
		if (iRet != SOCKET_ERROR)
		{
			if (iRet == ileng)
			{
				return 0;
			}
			else if ( iRet < ileng)			
			{
				ileng = ileng - iRet;
				iPos = iPos + iRet;
				continue;
			}
		}
		else
		{	

				iRet = errno;
				perror("A Error Occured while recv!!!");

				printf("Error When Sending WASERROR=%d",iRet);
			return -1;
		}
	}
}

void * ThreadWebWorker(void *pContext)
{
	pthread_detach (pthread_self());
	ThreadContext *pThrContext = (ThreadContext *)pContext;	
	void *pZContext = pThrContext->pZCntext;	
	long lSkop = 0;


	void* sksndMore=zmq_socket(pZContext,ZMQ_SUB);
	zmq_setsockopt(sksndMore,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_connect(sksndMore,"inproc://PubRspSend");


	void* sksndReq=zmq_socket(pZContext,ZMQ_PUSH);
	zmq_setsockopt(sksndReq,ZMQ_LINGER,&lSkop,sizeof(lSkop));
	zmq_connect(sksndReq,"inproc://ReqSnd");



	char RcvBuffer[1024] = {0};
	int RECVBUFF_LEN = 1024;
	struct timeval tv;
	int iRet =0; 
	int iError = SOCKET_ERROR;
	tv.tv_sec  = 5;						//5秒超时
	tv.tv_usec = 0;
	fd_set fsRead;
	while(true)
	{
		FD_ZERO(&fsRead);
		FD_SET(pThrContext->socket, &fsRead);
		iError = select(pThrContext->socket+1, &fsRead,NULL,NULL, NULL);	    //检测是否有数据, 有数据时读取, 解析
		if (iError > 0)                     //套接字准备就绪
		{
		  if(FD_ISSET(pThrContext->socket,&fsRead)) //套接字可读
		  {  
			memset(RcvBuffer,0,RECVBUFF_LEN);
			int iRecvLeng = recv(pThrContext->socket,&RcvBuffer[0],RECVBUFF_LEN,0);
			if ( -1 == iRecvLeng || 0 == iRecvLeng)		// 接收错误
			{
				iError = errno;
				perror("SOCKET recv error");
				close(pThrContext->socket);
				//错误或连接已断开
				zlog_error(g_server_cat,"SOCKET recv error,iRecvLeng=%d WASERROR=%ld ",iRecvLeng,iError);
				break;
			}
			if(iRecvLeng>0)		//接收处理数据
			{
				zlog_info(g_server_cat,"rev info from oss =%s\n",RcvBuffer);
				cJSON* root = cJSON_Parse(RcvBuffer); 
				if (root == NULL)
				{
					zlog_error(g_server_cat,"parse json string error: %s", cJSON_GetErrorPtr());
					continue;
				}
				else
				{
					char imei[50] = {0};
					if(cJSON_GetObjectItem(root,"imei") !=NULL && cJSON_GetObjectItem(root,"cmd") !=NULL)
					{
						zlog_info(g_server_cat,"parse json imei: %s", cJSON_GetObjectItem(root,"imei")->valuestring);
						
						sprintf(imei,"%s",cJSON_GetObjectItem(root,"imei")->valuestring);
						zlog_info(g_server_cat,"sub rsp imei = %s ",imei,strlen(imei));
						zmq_setsockopt(sksndMore, ZMQ_SUBSCRIBE, imei, strlen(imei));
						
					}
					else
					{
						continue;
					}

					zmq_msg_t msg;
					zlog_info(g_server_cat,"send json to msg deal threasd");
					int rc = zmq_msg_init_size (&msg, iRecvLeng);
					memcpy((char *)zmq_msg_data(&msg),RcvBuffer,iRecvLeng);
					rc = zmq_sendmsg(sksndReq,&msg,ZMQ_NOBLOCK);

					
					zlog_info(g_server_cat,"waitting json rsp from msg deal threasd......");
					zmq_pollitem_t pollitem;
					pollitem.socket = sksndMore;
					pollitem.fd = 0;
					pollitem.events = ZMQ_POLLIN;

					pollitem.revents = ZMQ_POLLOUT;
					zmq_poll(&pollitem,1,1000*10);
					if (pollitem.revents & ZMQ_POLLIN)
					{
						zlog_info(g_server_cat,"sub rsp from msg deal threasd ");
						zmq_msg_t msg;
						int rc = zmq_msg_init (&msg);
						rc = zmq_recvmsg (sksndMore, &msg, 0);
						zlog_info(g_server_cat,"imei = %s",(char *)zmq_msg_data(&msg));
						zmq_msg_close(&msg);
						rc = zmq_msg_init (&msg);
						rc = zmq_recvmsg (sksndMore, &msg, 0);

						Send2Web(pThrContext->socket,(char *)zmq_msg_data(&msg),zmq_msg_size(&msg));
					}
					zlog_debug(g_server_cat,"sdddddddddddddddddddddddddd");
				}	
			}
		  }	
		}
		else if( SOCKET_ERROR == iError )		//select出现错误
		{
			iError = errno;
			perror("SOCKET Recv error");
	
			
			close(pThrContext->socket);
			zlog_error(g_server_cat,"Select client socket error, WASERROR=%ld close connection",iError);
			iRet = RET_ERROR;
			break;
		}
		else //超时，无数据
		{
			zlog_debug(g_server_cat,"time out!");
		}
	}

	pthread_exit(NULL);
}



void handleSignal(int iSignal)
{
	switch(iSignal)
	{
	case SIGTERM:
			printf("%s is shutting down......\n",STR_APPNAME);
			g_isExit1 = true;
			break;
	default:
			break;
	}
}


