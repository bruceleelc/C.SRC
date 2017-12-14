

#ifndef CONFIGURE_H_
#define CONFIGURE_H_
#pragma once
#include "Tool.h"
#include <set>
#include <vector>
#include <string>
#include <map>
using namespace std;
	
#define  MAX_READER_NUM		5		
#define	 MAX_ROAD_NUM		8			
#define	 MAX_DIR_NUM		8			
#define  MAX_MODULE_NUM		8			
#define	 MAX_SERVER_NUM		8			
#pragma pack(1)
typedef const char* LPCTSTR;

typedef struct tagDbConf
{

	std::string strIp;
	std::string strUserID;
	std::string strPassword;
	std::string strServiceName;
	int dbconnNum;
	int port;
}DbConf;

#pragma pack()

//typedef const char* LPCTSTR;

#ifdef WIN32
#define uint32 UINT32
#else

#include <stdint.h>
#define sprintf_s snprintf
#define _stricmp strcasecmp
/*typedef uint32_t uint32;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;*/
#endif

class TiXmlNode;

class CConfigure
{
public:
	CConfigure(void);
	~CConfigure(void);
public:
	
	DbConf m_dbconf;
	int m_port;
	int m_webPort;
	std::string m_strrmqhostname;
    int m_irmqport;
	std::string m_strrmquser;
	std::string m_strrmqpasswd;
	std::string m_strrmqexchanage;
	std::string m_strrmqqueuename;

public:
	bool LoadBasicConfig(const char* lpszFileName);			//闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺€栫€氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲矗椤愶紕涓查柟椋庡厴閺佹捇寮妶鍡楊伓闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺€栫€氾拷
	bool LoadOtherConfig(const char* lpszFileName);			//闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺€栫€氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫€氾拷
	bool need_filter(int msg_type);							

	bool ParseDBInfo(TiXmlNode * ptemp);
	
	bool ParseMonitorInfo(TiXmlNode * ptemp);
	
	bool ParseLogConfInfo(TiXmlNode * ptemp);
	
	bool ParseCarrierInfo(TiXmlNode * ptemp);
	
	bool ParseDataForwardInfo(TiXmlNode * ptemp);
	
	bool ParseRTCarrier(TiXmlNode * ptemp);
	
	bool ParseHDCarrier(TiXmlNode * ptemp);

	bool ParseRTCarrierBak(TiXmlNode * ptemp);

	bool ParseOssInfo(TiXmlNode * ptemp);
	std::string getrmqhostname();
	// 取RMQ 端口号
	 int getrmqport();
	// 取RMQ 接受交换机名称
	 std::string getrmqexchanage();
	 std::string getrmqqueuename();
	 
	 
   LPCTSTR GetRmqUser();

   LPCTSTR GetRmqPasswd();

	
private:
	const char* GetLeafValue(TiXmlNode* pNode, const char* lpszName);
	
};

#endif 	/* CONFIGURE_H_ */
