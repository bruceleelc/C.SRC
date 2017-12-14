#pragma once
#include <vector>
#include <string>
//BSDsocket include files
#include   <sys/types.h>
#include   <sys/socket.h>
#include   <netinet/in.h>
#include   <arpa/inet.h>
#include   <map>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

using namespace std;


#ifdef WIN32
#define uint32 UINT32
#else

#include <stdint.h>
#define sprintf_s snprintf
#define _stricmp strcasecmp
typedef uint32_t uint32;
typedef const char* LPCTSTR;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
#endif

  

typedef struct tagDbConf
{

	std::string strIp;
	std::string strUserID;
	std::string strPassword;
	std::string strServiceName;
	int dbconnNum;
	int port;
}DbConf;


class TiXmlNode;

class CConfigure
{
public:
	CConfigure(void);
	~CConfigure(void);

public:
	// 读取配置文件，解析得到各参数
	bool LoadOptions(LPCTSTR lpszFileName);



	LPCTSTR GetRmqServerIP();

	int  GetRmqServerPort();

	LPCTSTR GetConSumerExchange();

	LPCTSTR GetQueueName();
	int GetConsumeIntervalTime();

	LPCTSTR GetRmqUser();

	LPCTSTR GetRmqPasswd();

protected:
	bool ParseDBInfo(TiXmlNode * ptemp);
	bool ParseRmqServerPara(TiXmlNode* pECM); 

private:
	LPCTSTR GetLeafValue(TiXmlNode* pNode, LPCTSTR lpszName);

public:
	DbConf m_dbconf;
	int    m_nChildThreadPoolCount;
	std::string m_strRmqServerIP;
	int m_iRmqServerPort;
	std::string m_strrmquser;
	std::string m_strrmqpasswd;
	std::string m_strConSumeExchange;
	std::string m_strQueueName;
	int		m_iConsumeIntervalTime;
};
