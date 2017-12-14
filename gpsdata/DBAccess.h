#pragma once
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <iconv.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <mysql/mysql.h>

#include <stdint.h>
#define sprintf_s snprintf
#define _stricmp strcasecmp

using namespace std;

/////入库结构体
typedef struct _gps_info_db
{
	//
	string          m_strDeviceID;         //设备序列号
	//DWORD           
	//
	string          m_strDeviceDate;       //设备上传的时间
	string          m_strS;                //数据有效位（A/V），A表示GPS数据是有效定位数据，V表示GPS数据是无效定位数据
	double          m_dfLatitude;          //纬度
	string          m_strD;                //纬度标志（N：北纬，S：南纬）
	double          m_dfLongitude;         //经度
	string          m_strG;                //经度标志（E：东经，W：西经）
	int             m_nSpeed;              //速度
	int             m_nDirection;          //方向
}_GPS_INFO_DB;



class DBAccess
{
public:
    DBAccess();
    virtual ~DBAccess();
    bool initDB(std::string strIP,std::string strDbName,std::string strUser,std::string strPassword);
    void GpsInsertDB( void* pData );
private:
    
    MYSQL               m_MysqlConn;
   
};
*/
#ifndef __CONNECTION_POOL_H__
#define __CONNECTION_POOL_H__
#include "Configure.h"
#include <pthread.h>
#include <iostream>
#include <vector>
#include <map>
#include "DataDef.h"
#include <mysql/mysql.h>
using namespace std;
#pragma pack(1)
/////入库结构体
typedef struct _gps_info_db
{
	char          m_strDeviceID[20];         //设备序列号

	char          m_strDeviceDate[20];       //设备上传的时间
	char          m_strS[2];                //数据有效位（A/V），A表示GPS数据是有效定位数据，V表示GPS数据是无效定位数据
	double          m_dfLatitude;          //纬度
	char          m_strD[2];                //纬度标志（N：北纬，S：南纬）
	double          m_dfLongitude;         //经度
	char          m_strG[2];                //经度标志（E：东经，W：西经）
	int             m_nSpeed;              //速度
	int             m_nDirection;          //方向
}_GPS_INFO_DB;

typedef struct _dev_status_db
{
	char          m_strDeviceID[20];         //设备序列号

	char          m_strDeviceDate[20];       //设备上传的时间
	char          	m_cVoltagelevel;          //电压等级,0:无电(关机);1:电量极低(不足以打电话发短信等);2:点亮很低(低电报警);3:电量低(可正常使用);4:电量中;5:电量高;6:电量极高
	char            m_cSignalstrength;        //GSM 信号强度,0:无信号;1:信号极弱;2:信号较弱;3:信号良好;4:信号强
	int          	m_nOilelecstat;          //油电状态,1:油电断开;0:油电接通
	int             m_nGpsstat;              //GPS定位状态,1:GPS 已定位;0:GPS 未定位
	int             m_nChargestat;          //充电状态,1:已接电源充电;0:未接电源充电
	int             m_nAccstat;              //ACC状态,1:ACC高;0:ACC低
	int             m_nProtectstat;          //设防状态,1:设防;0:撤防
}_DEV_STATUS_DB;

typedef struct _dev_gpswarn_db
{
	char          m_strDeviceID[20];         //设备序列号

	char          m_strDeviceDate[20];       //设备上传的时间
	char          m_strS[2];                //数据有效位（A/V），A表示GPS数据是有效定位数据，V表示GPS数据是无效定位数据
	double          m_dfLatitude;          //纬度
	char          m_strD[2];                //纬度标志（N：北纬，S：南纬）
	double          m_dfLongitude;         //经度
	char          m_strG[2];                //经度标志（E：东经，W：西经）
	int             m_nSpeed;              //速度
	int             m_nDirection;          //方向

	char 			m_cWarn;               //报警
	char          	m_cVoltagelevel;          //电压等级,0:无电(关机);1:电量极低(不足以打电话发短信等);2:点亮很低(低电报警);3:电量低(可正常使用);4:电量中;5:电量高;6:电量极高
	char            m_cSignalstrength;        //GSM 信号强度,0:无信号;1:信号极弱;2:信号较弱;3:信号良好;4:信号强
	int          	m_nOilelecstat;          //油电状态,1:油电断开;0:油电接通
	int             m_nGpsstat;              //GPS定位状态,1:GPS 已定位;0:GPS 未定位
	int             m_nChargestat;          //充电状态,1:已接电源充电;0:未接电源充电
	int             m_nAccstat;              //ACC状态,1:ACC高;0:ACC低
	int             m_nProtectstat;          //设防状态,1:设防;0:撤防
}_DEV_GPSWARN_DB;

#pragma pack()
#define  MYSQL_CONN_NUM_MAX_VALUE   1000
using namespace std;

enum _USE_STATUS
{
   US_USE = 0,
   US_IDLE = 1
};

typedef  struct _sConStatus
{
   void*  connAddr;
   int    useStatus;
}sConStatus;


class DBAccess
{
public:
    DBAccess();
    ~DBAccess();
public:
    void Init(DbConf &dbconf);//connection  pool init
    void* getOneConn();//get a connection
	void  retOneConn(void* pMysql);// return a connection
	int connDB();
	void disconn();
    //void  checkConn(); // check the connection if is alive
   
	void* createOneConn();
	void GpsInsertDB( void* pData );
	void DevStatusInsertDB( void* pData );
	void AlarmInsertDB( void* pData );
   
public:

   char m_szMysqlIp[100];
   char m_szUser[100];
   char m_szPwd[100];
   char m_szDbName[100];
   int  m_nMysqlPort;  
   int  m_nConnNum;
   SYSTEMTIME      stCurrent;
     
public:
    pthread_mutex_t m_mutex;
    vector<void*>  m_vectorConn;
    map<void*, int> m_mapVI;
    map<void*, void*> m_mapMysqlScs;
   
};

#endif