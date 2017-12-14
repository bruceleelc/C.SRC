#ifndef RECOGIZERSOCKET_H_
#define RECOGIZERSOCKET_H_


#pragma once
#include "RecogizerListener.h"
#include "VehicleInfoParser.h"
#include "DataDef.h"
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <list>
#include<iostream>  
#include<fstream>  
#include<string> 
#include "DBAccess.h"
#include "zmq.hpp"


#include<unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <fcntl.h>
using namespace std;
#define SOCKET_ERROR -1
typedef int	SOCKET;

#define RCVBUF_LENGTH		640 * 1024 + 640 * 1024
#define SNDBUF_LENGTH		640 * 1024

#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)

#define ESCAPE	0x7e		// 数据包头标记
#define ESCAPE_T	0x7d		// 数据包头标记
#define ESCAPE_2               0x02
#define ESCAPE_1               0x01

class RecogizerSocket
{
public:
	RecogizerSocket(void);
	~RecogizerSocket(void);
public:
	void Initial(const bool* pIsExit,ThreadContext* pContext,void* pZsoket1,void* pZsoket2,void* pZsoket3);
	int AutoConfSocket(void);
	int StartWork(void);
	int Send2Dev(const void* lpBuf, int nBufLen);
	void AnMonitorInfo(void);
	int ReadPicNumInfo(const char *FileName);
	
	int SavePicNumInfo(const char *FileName,char *PlateNumbuf);
	bool MsgEscape(char *srcData,int len,char *destData,unsigned int &flag ,int mode);
	char check(char *src,int len);
	vector<string> split(char *src,char *delimiters);
private:
	SYSTEMTIME      stCurrent;       
	const bool*		m_pIsExit;		
	PacketHead		m_Head;		
	DataDevState	m_DevState;	
	PacketHead		m_CarHead;		
	//DataCarInfo		m_CarInfo;		
    //PacketHead      m_CarNumHead;    
	PacketHead      m_CarRateHead;    
	
	

	SOCKET			m_socket;		
	char*			m_pRecvBuff;	
	unsigned long	m_ulRcvIndex;	
	ThreadContext*	m_pContext;		
	char			m_szHostIp[64];	
	void*	m_pPubSender;			
	
	void*	m_pRspSender;	
	
	void*	m_pGpsSender;

	//DataCarInforate m_carInforate;        
    //IdentifyNumList m_IdentifyNumList;   
	unsigned short olddaytime;
	time_t t_now ;					
	time_t t_last ;
};

#endif
