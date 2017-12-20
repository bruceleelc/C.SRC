#include "DBAccess.h"

#include "zlog.h"

extern zlog_category_t* g_server_cat;
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h> 
#include <iostream>                      
#include <memory>
#include <string>
#include <map>
#include <vector>



 

using namespace std;



DBAccess::DBAccess( )
{
	memset(m_szMysqlIp,0x0,sizeof(m_szMysqlIp));
	memset(m_szUser,0x0,sizeof(m_szUser));
	memset(m_szPwd,0x0,sizeof(m_szPwd));
	memset(m_szDbName,0x0,sizeof(m_szDbName));
	m_nMysqlPort = 3306;  
	m_nConnNum = 1;
}

DBAccess::~DBAccess( )
{
}

void* DBAccess::createOneConn()
{
     MYSQL*  mysql;
     mysql = mysql_init(0);
     if(mysql == NULL)
     {
		zlog_error(g_server_cat,"mysql_init fail*");
    return NULL;
	 }
	 char reconnectArg = 1;
	 mysql_options(mysql, MYSQL_OPT_RECONNECT, (char *)&reconnectArg);  
     if(mysql_real_connect(mysql,  m_szMysqlIp , m_szUser ,   m_szPwd,    m_szDbName  ,  m_nMysqlPort, NULL,0)==NULL)
     {
	 zlog_error(g_server_cat,"connect failure!");
     return NULL;
   }
     else
     {
		zlog_info(g_server_cat,"connect success!");
     }
     //
     return mysql;
 
}



void DBAccess::Init(DbConf &dbconf)
{
	pthread_mutex_init (&m_mutex, NULL);
    strcpy(m_szMysqlIp, dbconf.strIp.c_str());
    strcpy( m_szUser, dbconf.strUserID.c_str());
    strcpy(m_szPwd, dbconf.strPassword.c_str());
    strcpy(m_szDbName, dbconf.strServiceName.c_str());
    m_nMysqlPort = dbconf.port;  
	m_nConnNum = dbconf.dbconnNum; 
	
}
int DBAccess::connDB()
{
	zlog_info(g_server_cat,"conn db...... ,ip = %s,dbname = %s,dbpwd = %s,sid = %s",m_szMysqlIp,m_szUser,m_szPwd,m_szDbName);
	pthread_mutex_lock(&m_mutex);
    MYSQL*  mysql;
	zlog_info(g_server_cat,"m_nConnNum = %d",m_nConnNum);
    for(int i=0; i<m_nConnNum; i++)
    {
       mysql = (MYSQL*)this->createOneConn();
	   if(mysql == NULL)
	   {
			pthread_mutex_unlock(&m_mutex);
			return -1;
	   }
        
       // 
      sConStatus* scs = new sConStatus();
      scs->connAddr = mysql;
      scs->useStatus = US_IDLE;
      m_vectorConn.push_back(scs); 
      m_mapVI[scs] = i;
      m_mapMysqlScs[mysql] = scs;
  }
  pthread_mutex_unlock(&m_mutex);
  return m_nConnNum;
}

void DBAccess::disconn()
{
	zlog_info(g_server_cat,"disconn DB");
	pthread_mutex_lock(&m_mutex);
	int N = m_vectorConn.size();
	MYSQL*  mysql;
	for(int i=0; i< N; i++)
	{
			
			sConStatus* scs = (sConStatus*)m_vectorConn[i];
			mysql =(MYSQL*)(scs->connAddr);
			mysql_close(mysql);
			delete scs;
	}
	m_vectorConn.clear();
	m_mapVI.clear();
	m_mapMysqlScs.clear();
	pthread_mutex_unlock(&m_mutex);
}
//从连接池中取一个连接，同时，给它做一个标记，表明它已经被使用，防止别的线程再使用。
void* DBAccess::getOneConn()
{
    int N = m_vectorConn.size();
  for(int i=0; i< N; i++)
  {
		pthread_mutex_lock(&m_mutex);
        sConStatus* scs = (sConStatus*)m_vectorConn[i];
        if(scs->useStatus ==  US_IDLE)
        {
		   scs->useStatus = US_USE;
		   pthread_mutex_unlock(&m_mutex);
          return  scs->connAddr;
		} 
		pthread_mutex_unlock(&m_mutex);
  }
  //
  return NULL;
}

//把连接归还给连接池。同时，给它做一个标记，表明它是空闲的，可以使用。
void  DBAccess::retOneConn(void* pMysql)
{
  if(!pMysql)
    return;
  // 
  map<void*, void*>::iterator  it1;
  map<void*, int>::iterator it2;
 
  pthread_mutex_lock(&m_mutex);
 
  it1 = m_mapMysqlScs.find(pMysql);
  if(it1 == m_mapMysqlScs.end())
  {
	  pthread_mutex_unlock(&m_mutex);
	  return;
  }

  it2 = m_mapVI.find(it1->second);
  if(it2 == m_mapVI.end())
  {
	  pthread_mutex_unlock(&m_mutex);
	  return;
  }
      
  int nInx = it2->second;

  sConStatus* scs = (sConStatus*) m_vectorConn[nInx];
  scs->useStatus = US_IDLE;
  pthread_mutex_unlock(&m_mutex);
  
}



bool DBAccess::GpsInsertDB( void* pData )
{
	if (NULL == pData)
	{
		zlog_error(g_server_cat,"pData NULL !");
		return true;
	}
	vector<string> vec = split((char *)pData,',');
 	time_t t;
	t = atol(vec[1].c_str());
	struct tm *p;
	p=gmtime(&t);
	char s[20] = {0};  
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", p);
	char strSql[1024] = {0};
	if (vec[7] == "1")
	{
		if (13 > vec.size())
		{
			zlog_error(g_server_cat,"msg error :%s", pData);
			return true;
		}
		sprintf(strSql, "INSERT INTO tb_gpsdata_stu(deviceid, gpsdate,insertdate, battery, signal, tcard, \
						  event, event_data, type, latitude, longitude, alt, speed, direction,\
		                  mmac, msignal, mssid, mac, wifi_signal, ssid, serverip, cdma, network, sid, \
						  nid, bid, lng, lat, jizhan_signal, mcc, mnc, lac, cellid) \
						  values ('%s', '%s',now(), %s, '%s', %s, %s, '%s', %s, %f, %f, %s, %f, %s, \
						  ' ', ' ', ' ', ' ', ' ', ' ', ' ', -1, ' ', ' ', ' ', ' ',9999,9999, ' ', ' ', -1, ' ', ' ')", 
						  vec[0].c_str(),
						  s,
						  vec[2].c_str(),
						  vec[3].c_str(), 
						  vec[4].c_str(),
						  vec[5].c_str(),
						  vec[6].c_str(),
						  vec[7].c_str(),
						  atof(vec[8].c_str())/1000000,
						  atof(vec[9].c_str())/1000000,
						  vec[10].c_str(),
						  atof(vec[11].c_str())/10000000,
						  vec[12].c_str()
						  );
	}
	else if (vec[7] == "2")
	{
		if (14 > vec.size())
		{
			zlog_error(g_server_cat,"msg error :%s", pData);
			return true;
		}
		sprintf(strSql, "INSERT INTO tb_gpsdata_stu(deviceid, gpsdate,insertdate, battery, signal, tcard, \
						  event, event_data, type, latitude, longitude, alt, speed, direction,\
		                  mmac, msignal, mssid, mac, wifi_signal, ssid, serverip, cdma, network, sid, \
						  nid, bid, lng, lat, jizhan_signal, mcc, mnc, lac, cellid) \
						  values ('%s', '%s',now(), %s, '%s', %s, %s, '%s', %s, 9999, 9999, 999999, -1, 9999, \
						  '%s', '%s', '%s','%s', '%s', '%s', ' ', -1, ' ', ' ', ' ', ' ',9999, 9999, ' ', ' ', -1, ' ', ' ')", 
						  vec[0].c_str(),
						  s,
						  vec[2].c_str(),
						  vec[3].c_str(), 
						  vec[4].c_str(),
						  vec[5].c_str(),
						  vec[6].c_str(),
						  vec[7].c_str(),
						  vec[8].c_str(),
						  vec[9].c_str(),
						  vec[10].c_str(),
						  vec[11].c_str(),
						  vec[12].c_str(),
						  vec[13].c_str()
						  );
	}
	else if (vec[7] == "3")
	{
		if (vec[9] == "0")
		{
			if (15 > vec.size())
			{
				zlog_error(g_server_cat,"msg error :%s", pData);
				return true;
			}
			sprintf(strSql, "INSERT INTO tb_gpsdata_stu(deviceid, gpsdate,insertdate, battery, signal, tcard, \
						  event, event_data, type, latitude, longitude, alt, speed, direction,\
		                  mmac, msignal, mssid, mac, wifi_signal, ssid, serverip, cdma, network, sid, \
						  nid, bid, lng, lat, jizhan_signal, mcc, mnc, lac, cellid) \
						  values ('%s', '%s',now(), %s, '%s', %s, %s, '%s', %s, 9999, 9999, 999999, -1, 9999, \
						  ' ', ' ', ' ',' ', ' ', ' ', '%s', %s, '%s', ' ', ' ', ' ',9999, 9999, '%s', '%s', %s, '%s', '%s')", 
						  vec[0].c_str(),
						  s,
						  vec[2].c_str(),
						  vec[3].c_str(), 
						  vec[4].c_str(),
						  vec[5].c_str(),
						  vec[6].c_str(),
						  vec[7].c_str(),
						  vec[8].c_str(),
						  vec[9].c_str(),
						  vec[10].c_str(),
						  vec[15].c_str(),
						  vec[11].c_str(),
						  vec[12].c_str(),
						  vec[13].c_str(),
						  vec[14].c_str()
						  );
		}
		else
		{
			if (17 > vec.size())
			{
				zlog_error(g_server_cat,"msg error :%s", pData);
				return true;
			}
			sprintf(strSql, "INSERT INTO tb_gpsdata_stu(deviceid, gpsdate,insertdate, battery, signal, tcard, \
						  event, event_data, type, latitude, longitude, alt, speed, direction,\
		                  mmac, msignal, mssid, mac, wifi_signal, ssid, serverip, cdma, network, sid, \
						  nid, bid, lng, lat, jizhan_signal, mcc, mnc, lac, cellid) \
						  values ('%s', '%s',now(), %s, '%s', %s, %s, '%s', %s, 9999, 9999, 999999, -1, 9999, \
						  ' ', ' ', ' ',' ', ' ', ' ', '%s', %s, '%s', '%s', '%s', '%s', %s, %s, '%s', ' ', -1, ' ', ' ')", 
						  vec[0].c_str(),
						  s,
						  vec[2].c_str(),
						  vec[3].c_str(), 
						  vec[4].c_str(),
						  vec[5].c_str(),
						  vec[6].c_str(),
						  vec[7].c_str(),
						  vec[8].c_str(),
						  vec[9].c_str(),
						  vec[10].c_str(),
						  vec[11].c_str(),
						  vec[12].c_str(),
						  vec[13].c_str(),
						  vec[14].c_str(),
						  vec[15].c_str(),
						  vec[16].c_str()
						  );
		}
		
	}

	zlog_info(g_server_cat,"Sql: %s", strSql);
	int res;
	MYSQL *mysql = NULL;;
	while(NULL == mysql)
	{
		mysql = (MYSQL *)getOneConn();
		usleep(10);
	}
	res = mysql_query(mysql, strSql);

	if (res)
	{
		zlog_error(g_server_cat,"Insert Failed:%d, desc: %s", mysql_errno(mysql), mysql_error(mysql));
		disconn();
		connDB();
		return false;
	}
	else
	{
		zlog_info(g_server_cat,"Insert tb_gpsdata_stu success");
		retOneConn(mysql);
	}
	return true;
}


bool DBAccess::AlarmInsertDB( void* pData )
{
	if (NULL == pData)
	{
		zlog_error(g_server_cat,"pData NULL !");
		return true;
	}
	_DEV_GPSWARN_DB *pAlarmData = (_DEV_GPSWARN_DB*)(pData);


	char strSql[1024] = {0};
	sprintf(strSql, "INSERT INTO tb_gps_alarm(deviceid, gpsdate,insertdate, s, latitude, d, longitude, g, speed, direction,  \
		   				  voltage_level, signal_strength, oil_elec_status, gps_status, charge_status, acc_status, protect_status,alarm_type) \
		                  values ('%s', '%s',now(), '%s', %f, '%s', %f,'%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", 
						  pAlarmData->m_strDeviceID,
						  pAlarmData->m_strDeviceDate,
						  pAlarmData->m_strS,
						  pAlarmData->m_dfLatitude,
						  pAlarmData->m_strD, 
						  pAlarmData->m_dfLongitude,
						  pAlarmData->m_strG,
						  pAlarmData->m_nSpeed,
						  pAlarmData->m_nDirection,
						  pAlarmData->m_cVoltagelevel,
						  pAlarmData->m_cSignalstrength,
						  pAlarmData->m_nOilelecstat,
						  pAlarmData->m_nGpsstat,
						  pAlarmData->m_nChargestat,
						  pAlarmData->m_nAccstat,
						  pAlarmData->m_nProtectstat,
						  pAlarmData->m_cWarn
						  );

	zlog_info(g_server_cat,"Sql: %s", strSql);
	int res;
	MYSQL *mysql = NULL;;
	while(NULL == mysql)
	{
		mysql = (MYSQL *)getOneConn();
		usleep(10);
	}
	res = mysql_query(mysql, strSql);

	if (res)
	{
		zlog_error(g_server_cat,"Insert Failed:%d, desc: %s", mysql_errno(mysql), mysql_error(mysql));
		disconn();
		connDB();
		return false;
		
	}
	else
	{
		zlog_info(g_server_cat,"Insert tb_gps_alarm success");
		retOneConn(mysql);
	}
	return true;
}

bool DBAccess::DevStatusInsertDB( void* pData )
{
	if (NULL == pData)
	{
		zlog_error(g_server_cat,"pData NULL !");
		return true;
	}
	_DEV_STATUS_DB *pDevStatData = (_DEV_STATUS_DB*)(pData);

	MYSQL *mysql = NULL;;
	while(NULL == mysql)
	{
		mysql = (MYSQL *)getOneConn();
		usleep(10);
	}
	int res;
	

	char strSql[1024] = {0};
	sprintf(strSql, "update tb_dev_status set heartdate = '%s', insertdate = now(), voltage_level = %d, signal_strength = %d, \
						  oil_elec_status = %d, gps_status = %d, charge_status = %d, acc_status = %d, protect_status = %d \
		                  where deviceid = '%s'", 
						  pDevStatData->m_strDeviceDate,
						  pDevStatData->m_cVoltagelevel,
						  pDevStatData->m_cSignalstrength,
						  pDevStatData->m_nOilelecstat,
						  pDevStatData->m_nGpsstat,
						  pDevStatData->m_nChargestat,
						  pDevStatData->m_nAccstat,
						  pDevStatData->m_nProtectstat,
						  pDevStatData->m_strDeviceID
						  );
	zlog_info(g_server_cat,"Sql: %s", strSql);
	res = mysql_query(mysql, strSql);
	if (res)
	{
		zlog_error(g_server_cat,"update Failed:%d, desc: %s", mysql_errno(mysql), mysql_error(mysql));
		disconn();
		connDB();
		
		return false;
		
		
	}
	else
	{
		long updataNum = (long) mysql_affected_rows(mysql);
		if (0 == updataNum)
		{
			memset(strSql,0x0,1024);
			sprintf(strSql, "INSERT INTO tb_dev_status(deviceid, heartdate,insertdate, voltage_level, signal_strength, oil_elec_status, gps_status, charge_status, acc_status, protect_status) \
								  values ('%s', '%s',now(), %d, %d, %d, %d, %d, %d, %d)", 
								  pDevStatData->m_strDeviceID,
								  pDevStatData->m_strDeviceDate,
								  pDevStatData->m_cVoltagelevel,
								  pDevStatData->m_cSignalstrength,
								  pDevStatData->m_nOilelecstat,
								  pDevStatData->m_nGpsstat,
								  pDevStatData->m_nChargestat,
								  pDevStatData->m_nAccstat,
								  pDevStatData->m_nProtectstat
								  );
		
			zlog_info(g_server_cat,"Sql: %s", strSql);
			
			
			res = mysql_query(mysql, strSql);
		
			if (res)
			{
				zlog_error(g_server_cat,"Insert Failed:%d, desc: %s", mysql_errno(mysql), mysql_error(mysql));
				disconn();
				connDB();
				
				return false;
				
			}
			else
			{
				zlog_info(g_server_cat,"Insert tb_dev_status success");
				retOneConn(mysql);
			}
		}
		else
		{
			zlog_info(g_server_cat,"update tb_dev_status success");
			retOneConn(mysql);
		}
		
	}
	return true;
	
}
