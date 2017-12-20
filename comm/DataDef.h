//********************************************************
//文件:DataDef.h
//说明:系统通用数据结构声明 所有进行通讯的工程均需包含此头文件
//作者:zdx
//创建时间:2011-08-03 09:00
//----------------------------------------------------------
//修改记录:
//修改者:
//修改日期:
//修改内容:
//********************************************************
#ifndef DATADEF_H_
#define DATADEF_H_


#pragma once

#include<time.h>

//文件夹分隔符
#ifndef linux
//windows
#define DIR_SLASH '\\'
#else
//linux
#define DIR_SLASH '/'
#endif



//可用于linux定义
#ifdef linux

#include <sys/sysinfo.h>
#define MAX_PATH	 1024
typedef long long				__int64;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
}SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

void GetLocalTime(SYSTEMTIME* lpSystemTime);

void SystemTime2Time_T(LPSYSTEMTIME pSysTime,time_t* pTime);

unsigned long  GetTickCount(void);

#endif
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

//add by zdx zdx
#ifndef ARM_LINUX
#define HIGH_WATERMARK		500
#else
#define HIGH_WATERMARK		25
#endif

//系统内探测包等
#define SYSTEM_OK			0x1001      //成功
#define SYSTEM_ERROR		0x1002      //失败
#define SYSTEM_HEART		0x1003      //心跳

//基本数据包-类型
#define DEV_EV19	0x1101		//过车记录
#define DATA_GPS_LOCATION		0x2101		//GPS位置数据
#define DATA_GPS_ALARM		0x2102      //GPS报警数据
#define DATA_DEV_STATUS		0x2103		//设备状态数据

#define DEV_Q005_024_01	    0x1102		//过车记录
#define DATA_STU_GPS_LOCATION	0x3101		//GPS位置数据
#define DATA_STU_GPS_ALARM		0x3102      //GPS报警数据
#define DATA_STU_DEV_STATUS		0x3103		//设备状态数据

// ***BEGIN***  Add the real time msg  2012-11-06 10:29:29 add by lichuang
//实时数据包-类型
#define RTM_VEHICLE_REC		0x1107       //实时过车记录             
#define RTM_TAG_REC			0x1108      //实时标签记录             
// ****END****  Add the real time msg  2012-11-06 10:29:29 add by lichuang
#define RTM_USER_REC		0x1109      //实时用户区数据




//基本数据包-状态包类型
#define DEV_STATE			0x1200		//设备状态
#define DEV_STATESET		0x1201      //设备状态集合包
#define DEV_CAMERA			0x1202      //车牌识别相机
#define DEV_RECOGIZER		0x1203      //车牌识别器
#define DEV_RFID			0x1204      //RFID采集设备


//Proto包类型
//#define PROTO_VEHICLE_REC	0x1301		//只含图片和索引的过车记录(Proto)
#define PROTO_VEHICLE_REC	1309		//只含图片和索引的过车记录(Proto)
#define PROTO_TAG_REC		1302		//标签实时记录(Proto)
#define PROTO_TAG_REC_EX	1303      //标签扩展记录(Proto)
#define PROTO_MATCH_REC		1304		//匹配结果(Proto)
#define PROTO_DEV_STATES	1305		//设备状态集合

#define PROTO_OBD_REC	    1306		//OBD记录(Proto)
#define PROTO_USER_REC	    1307		//用户区记录(Proto)
#define PROTO_VEHICLE_DATA 1308  // 不含图片的过车记录(Proto)
//监控相关
#define STATION_STATE 1501          //基站状态消息
#define VEH_DISCRIMINATION 1502     //基站每天相机数据识别率
#define VEH_MATCHRATE 1503          //基站每天相机数据匹配率
#define STATION_PICNUM 1504         //基站当天上传相机数据总量

//监控相关
#define STATION_STATE_PRO 0x1501          //基站状态消息
#define VEH_DISCRIMINATION_PRO 0x1502     //基站每天相机数据识别率
#define VEH_MATCHRATE_PRO 0x1503          //基站每天相机数据匹配率
#define STATION_PICNUM_PRO 0x1504         //基站当天上传相机数据总量


//会话等其他相关
#define QUERY_SAFE_KEYS		0x1401			//请求验证密钥
#define QUERY_SAFE_VALID	0x1402			//请求验证
#define QUERY_CHECK			0x1403			//检查连接
#define QUERY_LOG_ON		0x1404			//注册
#define QUERY_LOG_OFF		0x1405			//注销

/* 
modify by zdx  2012-06-20
新数据结构使用，设备类型不变，原定义除去
*/
//设备类型
#define DEVTYPE_MATCHINE   0	//卡口机器本身
#define DEVTYPE_RECOGNISER 1	//抓拍摄像机或车牌识别器
#define DEVTYPE_CAMERA     1	//抓拍摄像机或车牌识别器
#define DEVTYPE_READER     2	//RFID电子车牌读写器


//设备状态
#define DEVSTATE_UNKNOW		0			//不明确
#define DEVSTATE_OK			1           //状态正常
#define DEVSTATE_ERROR		2			//状态异常

//服务器应答代码
#define SVRREP_DB_DISCONNECT	0x00000001 		//数据库连接失败
#define SVRREP_INVALID_DATA		0x00000002		//无效的，非法的数据
#define SVRREP_NAS_DISCONNECT 	0x00000003		//NAS服务器连接失败


//进程ID定义
#define PROC_ID_PLATE		0x0000000000010000			//车牌识别
#define PROC_ID_ETAGE		0x0000000000020000			//电子车牌
#define PROC_ID_GENER		0x0000000000030000			//综合逻辑处理
#define PROC_ID_CARRIER		0x0000000000040000			//传输程序
#define PROC_ID_Security	0x0000000000050000		//设备安全(加密狗)

#define PROC_ID_OBD         0x0000000000060000      //OBD


#define PACKET_FLAG			0xC3E7C3E7		//固定包头标志


#pragma pack(1)							
//网络通讯包包头
typedef struct tagPacketHead{
	WORD		wDevType;				//数据类型
	WORD		wMsgType;
	char		szImei[50];				
}PacketHead;


//------------------------以下部分用于车牌识别采集内部以及与综合逻辑处理之间的通讯----------------------------------
//统一设备状态
typedef struct tagDataDevState{
	BYTE        ucDevDirection;		//方向
	BYTE		ucDevRoadNo;		//车道号
	BYTE        ucDevType;			//设备类型
	BYTE        ucDevSerial;		//设备序号
	BYTE        ucDevState;			//设备状态
	time_t		tDevTime;			//设备状态更新时间
	/* add by zdx 2012-09-07   begin */
	char		szNetwork[32];		//设备网络信息 如（192.168.15.33:9977）
	char		szStartTime[32];	//设备接入时间 如（2010-09-20 16:44:58）
	/* add by zdx 2012-09-07   end */
}DataDevState;

//车牌数据包
typedef struct tagDataPlateInfo{
	char		szRecordID[28];			//过车流水号
	char		szPlateNum[16];			//车牌号
	unsigned char ucPlateColor;			//车牌颜色
}DataPlateInfo;


//------------------------以下部分用于电子标签采集内部以及与综合逻辑处理之间的通讯----------------------------------
typedef struct tagDataEtag{
	unsigned char	ucRoadNum;					//车道号
	char			szTagEPC[64];				//EPC长度
	unsigned char	EpcData[16];				//EPC原始数据
	unsigned char	ucEpcLeng;					//EPC长度
	unsigned char	TidData[16];				//TID原始数据
	unsigned char	ucTidLeng;
	unsigned char	ucUserData[64];				//用户区数据
	unsigned char	ucUserLeng;					//用户区数据长度
}DataEtag;


//-----------------------以下部分用于数据传输程序--------------------------------------------------------------------
//服务器状态检查包
typedef struct tagCheckPack{
	PacketHead	PKhead;				//包头
	char	szWords[10];				//包体内容
}CheckPack;

//服务器返回统一结构
typedef struct tagSvrRep{
	unsigned char ucResult;			//执行结果
	DWORD		  dwError;				//错误编号
	char*		  pDescrip;			//错误描述
}SvrRep;

//用与RTDateserver 统计每天过车数量
typedef struct RTStationPicNum
{
	unsigned int StationNo;   //基站编号
	unsigned long   PicNumTotal; //图片数据数量
	unsigned char CollectTime[32]; //统计时间 如（2015-09-20 16:44:58）
	unsigned int  IntervalTime; //采集时间间隔(单位：秒)
}DataStationPicNum;

#pragma pack()


#endif	/* DATADEF_H_ */
