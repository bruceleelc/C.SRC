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

#define SOCKET_ERROR -1
typedef int	SOCKET;

#define RCVBUF_LENGTH		640 * 1024 + 640 * 1024
#define SNDBUF_LENGTH		640 * 1024

#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)

#pragma pack(1)

#pragma pack()

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
