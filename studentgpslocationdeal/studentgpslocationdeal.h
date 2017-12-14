// TagRecDeal.h : 定义控制台应用程序的入口点头文件。
//
#pragma once
#include <string>
#include "DBAccess.h"
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "utils.h"
#include "Tool.h"
#include "AppInfo.h"
#include "GetProcessInfo.h"
#include "Lnxprocdef.h"
#include "zlog.h"


#define PACKET_FLAG         0x5aa5
#define NODE_PROCESS_REQ    0x04
#define NODE_DEVICE_REQ     0x05
#define NODE_STATINFO_REQ 0x06
#define NODE_REALTIME_REQ   0x07

#define SEND_BUFF           2048
#define RECEIVE_BUFF_SNMP   1024
#define TIME_LEN            32
#define SERV_PORT_LEN       10
#define NETCARD_LEN         64
#define PID_LEN             10
#define BYTES_LEN           10
#define SYSINFO_LEN         128
#define INTERVAL_TIME       10

#define VERSION_NUM         "NJITS-TagRecDealV2.1.0"
#define PROCESSNAME         "tagrecdeal"

#define ERROR_CATEGORY_MASK (0xFF00)  //rabbitmq connection error
#define ROUTINGKEY_LEN  100
#define MAGIC_NUM    	0xc3e7c3e7


// ****END****  Add the function of Monitor Collect Server  2012-09-18 16:40:42 add by lihao

typedef struct _HeartBeatData
{
    bool bismsgsrvfailure;         //MSGSrv心跳包是否无响应
    uint64_t llhb_nexttime;         //下次应该发送心跳包时间
    uint64_t llhb_lastrecvtime;     //上次收到心跳包的时间
    uint64_t llhb_lastsenttime;     //上次发送心跳包的时间
} HeartBeatData;

typedef std::pair<std::string, int> VehInfo;

typedef std::map<VehInfo, int> PlateNumMap;
typedef std::map<VehInfo, int>::iterator PlateNumMapIterator;
