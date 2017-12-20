
#include <stdio.h>
#include "RecogizerSocket.h"
#include "VehicleInfoParser.h"
#include "Configure.h"
#include "zlog.h"
#include "cJSON.h"
extern CConfigure g_configure;					
extern zlog_category_t* g_server_cat;
extern DBAccess g_dbaccess;

RecogizerSocket::RecogizerSocket(void)
{
	m_pIsExit = NULL;
	m_pRecvBuff = NULL;
	m_ulRcvIndex = 0;
	memset(&m_Head,0x00,sizeof(PacketHead));
	memset(&m_CarHead,0x00,sizeof(PacketHead));
	memset(&m_DevState,0x00,sizeof(DataDevState));
	memset(m_szHostIp,0x00,sizeof(m_szHostIp));
	//memset(&m_CarInfo,0x00,sizeof(DataCarInfo));

}

RecogizerSocket::~RecogizerSocket(void)
{
 	if (NULL != m_pRecvBuff)			
 	{
 		delete [] m_pRecvBuff;
	}

	close(m_socket);

	m_pContext = NULL;

}


void RecogizerSocket::Initial( const bool* pIsExit,ThreadContext* pContext,void* pZsoket1,void* pZsoket2,void* pZsoket3)
{

	SYSTEMTIME stCurrent;
	in_addr  addr;

	addr.s_addr = pContext->ulIP;
	char * szIp = inet_ntoa(addr);
	sprintf(m_szHostIp,"%s",szIp);		

	m_pIsExit = pIsExit;
	m_pContext = pContext;
	m_socket =  pContext->socket;

	m_pPubSender = pZsoket1;
	m_pRspSender = pZsoket2;
	m_pGpsSender = pZsoket3;



	m_pRecvBuff = new char[RCVBUF_LENGTH];
	m_DevState.ucDevType == DEVTYPE_CAMERA;
	m_DevState.ucDevDirection = 1;


	
    
	GetLocalTime(&stCurrent);
	sprintf(m_DevState.szNetwork,"%s:%d",szIp,pContext->usPort);
	sprintf(m_DevState.szStartTime,"%04d-%02d-%02d %02d:%02d:%02d",stCurrent.wYear,stCurrent.wMonth,
		stCurrent.wDay,stCurrent.wHour,
		stCurrent.wMinute,stCurrent.wSecond);
	
	
}

int RecogizerSocket::AutoConfSocket( void )
{
	unsigned long ulEnable = 1;
	fcntl(m_socket,F_SETFL,O_NONBLOCK);

	int nSendBufLength = 32 * 1024;
	setsockopt(m_socket,SOL_SOCKET,SO_SNDBUF,(char*)&nSendBufLength,sizeof(int));

	int bNoDelay = 1;
	setsockopt(m_socket,IPPROTO_TCP,TCP_NODELAY,(char*)&bNoDelay, sizeof(BOOL));

	int bKeepAlive = 1;
	setsockopt(m_socket,SOL_SOCKET,SO_KEEPALIVE,(char*)&bKeepAlive, sizeof(BOOL));

	int keepIdle = 1;	
	int keepInterval = 1;	
	int keepCount = 3; 		
	setsockopt(m_socket, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
	setsockopt(m_socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(m_socket, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

	return 0;
}



int RecogizerSocket::StartWork( void )
{
	int iError = SOCKET_ERROR;
	int iRet = 0;							
	int iHeadLeng = sizeof(PacketHead);		
	//int iInfoLeng = sizeof(DataCarInfo);	
	int iMoveSize = 0;						
	int	itype = 0;							
	VehicleInfoParser vehParser;			

	
	olddaytime = 0;
	t_now = 0;								
	t_last = 0;

	PacketHead header;
	memset(&header,0x0,sizeof(PacketHead));
	header.wDevType = DEV_EV19;
	fd_set fsRead;  

	struct timeval tv;
	
	zmq_pollitem_t pollitem;
	pollitem.socket = m_pPubSender;
	pollitem.fd = 0;
	pollitem.events = ZMQ_POLLIN;

	tv.tv_sec  = 0;								
	tv.tv_usec = 10000; 
	while (!(*m_pIsExit))
	{
		pollitem.revents = ZMQ_POLLOUT;
		zmq_poll(&pollitem,1,10);
		if (pollitem.revents & ZMQ_POLLIN)
		{
			zlog_info(g_server_cat,"sub cmd req");
			zmq_msg_t msg;
			int rc = zmq_msg_init (&msg);
			rc = zmq_recvmsg (m_pPubSender, &msg, 0);
			zlog_info(g_server_cat,"imei = %s",(char *)zmq_msg_data(&msg));
			zmq_msg_close(&msg);
			rc = zmq_msg_init (&msg);
			rc = zmq_recvmsg (m_pPubSender, &msg, 0);

			zlog_info(g_server_cat,"cmd = %s",(char *)zmq_msg_data(&msg));
			
			unsigned char sndData[1024]={0};
			vehParser.SetCMDData((char *)zmq_msg_data(&msg),zmq_msg_size(&msg),sndData);
			iError = Send2Dev(sndData,zmq_msg_size(&msg)+17);				
			if (-1 == iError)												
			{
				iRet = -2;
				break;
			}
			
		}
		
		FD_ZERO(&fsRead);  
		FD_SET(m_socket, &fsRead); 
		iError = select(m_socket+1, &fsRead,NULL, NULL, &tv);
		if (iError > 0)
		{
			if (RCVBUF_LENGTH <= m_ulRcvIndex)				
			{
				zlog_info(g_server_cat,"Recv Error, Buffer is full, Clear Data,Thid %ld IP %s",m_pContext->lThid,m_szHostIp);
				m_ulRcvIndex = 0;
			}

			int iRecvLeng = recv(m_socket,&m_pRecvBuff[m_ulRcvIndex],RCVBUF_LENGTH-m_ulRcvIndex,0);
			if ( SOCKET_ERROR == iRecvLeng || 0 == iRecvLeng)
			{

				iError = errno;
				zlog_info(g_server_cat,"SOCKET recv error,");

				zlog_info(g_server_cat,"SOCKET recv error,iRecvLeng=%d WASERROR=%ld ,Thid %ld IP %s",iRecvLeng,iError,m_pContext->lThid,m_szHostIp);

				iRet = -1;
				break;
			}
			zlog_info(g_server_cat,"recv data from dev ....m_ulRcvIndex = %d....iRecvLeng = %d.............",m_ulRcvIndex,iRecvLeng);
			hzlog_debug(g_server_cat,&m_pRecvBuff[m_ulRcvIndex],iRecvLeng);
			m_ulRcvIndex += iRecvLeng;
			
			zlog_info(g_server_cat,"recv buf data ........m_ulRcvIndex = %d.............",m_ulRcvIndex);
			hzlog_debug(g_server_cat,m_pRecvBuff,iRecvLeng);
			iMoveSize = 0;
			itype = ID_ERR;	
			iRet =0;
			int iResult = 0;
			
			
			while(vehParser.ParserData((unsigned char *)m_pRecvBuff,m_ulRcvIndex,&itype,&iMoveSize))		
			{  
				zlog_info(g_server_cat,"iMoveSize = %d ",iMoveSize);
			
				if (iMoveSize > 0 && iMoveSize <= m_ulRcvIndex && iMoveSize  < RCVBUF_LENGTH)	
				{
					m_ulRcvIndex = m_ulRcvIndex - iMoveSize;	
					memmove(m_pRecvBuff,&m_pRecvBuff[iMoveSize],m_ulRcvIndex);	
				}
				else if(iMoveSize == 0 && m_ulRcvIndex > 0)	
				{
					zlog_info(g_server_cat,"ParserData fail not enough data,recved %ld ParseRet=%d",m_ulRcvIndex,iMoveSize);
					break;
				}
				else
				{
					iRet = -4;								
					zlog_info(g_server_cat,"ParserData Error,recved %ld ParseRet=%d",m_ulRcvIndex,iMoveSize);
					break;
				}

				if (itype == ID_ERR)				
				{
					break;
				}
				else if (itype == ID_HEART)			
				{
					vehParser.SetRelpyData(ID_HEART);	
													
					iError = Send2Dev(&vehParser.m_ReplyHeart,sizeof(ReplyHeart));	
					if (-1 == iError)												
					{
						iRet = -2;
						break;
					}

					header.wMsgType = DATA_DEV_STATUS;
					zmq_msg_t msg;
					
					int rc = zmq_msg_init_size (&msg, sizeof(header)+sizeof(vehParser.m_devStat));
					memcpy((char *)zmq_msg_data(&msg),&header,sizeof(header));
					memcpy((char *)zmq_msg_data(&msg)+sizeof(header),&vehParser.m_devStat,sizeof(vehParser.m_devStat));
					rc = zmq_sendmsg(m_pGpsSender,&msg,ZMQ_NOBLOCK);
					//g_dbaccess.DevStatusInsertDB(&vehParser.m_devStat);	
					
				}
				else if(itype == ID_LOGIN)		
				{
					vehParser.SetRelpyData(ID_LOGIN);	
					memcpy(header.szImei,vehParser.m_szIMEI,strlen(vehParser.m_szIMEI));
					zlog_info(g_server_cat,"sub imei = %s ",vehParser.m_szIMEI);
					zmq_setsockopt(m_pPubSender, ZMQ_SUBSCRIBE, vehParser.m_szIMEI, strlen(vehParser.m_szIMEI));	
					iError = Send2Dev(&vehParser.m_ReplyHeart,sizeof(ReplyHeart));		
					if (-1 == iError)															
					{
						iRet = -2;
						break;
					}
				}
				else if(itype == ID_TIME)		
				{
				    vehParser.SetRelpyTimeData(ID_TIME);				
					iError = Send2Dev(&vehParser.m_ReplyTime,sizeof(ReplyTime));			
					if (-1 == iError)														
					{
						iRet = -2;
						break;
					}
				}
				else if(itype == ID_GPS)	
				{
					header.wMsgType = DATA_GPS_LOCATION;
					zmq_msg_t msg;
					
					int rc = zmq_msg_init_size (&msg, sizeof(header)+sizeof(vehParser.m_gpsData));
					memcpy((char *)zmq_msg_data(&msg),&header,sizeof(header));
					memcpy((char *)zmq_msg_data(&msg)+sizeof(header),&vehParser.m_gpsData,sizeof(vehParser.m_gpsData));
					rc = zmq_sendmsg(m_pGpsSender,&msg,ZMQ_NOBLOCK);

					//g_dbaccess.GpsInsertDB(&vehParser.m_gpsData);
				}
				else if(itype == ID_WARN)			
				{
					header.wMsgType = DATA_GPS_ALARM;
					zmq_msg_t msg;
					
					int rc = zmq_msg_init_size (&msg, sizeof(header)+sizeof(vehParser.m_gpsWarn));
					memcpy((char *)zmq_msg_data(&msg),&header,sizeof(header));
					memcpy((char *)zmq_msg_data(&msg)+sizeof(header),&vehParser.m_gpsWarn,sizeof(vehParser.m_gpsWarn));
					rc = zmq_sendmsg(m_pGpsSender,&msg,ZMQ_NOBLOCK);
					//g_dbaccess.AlarmInsertDB(&vehParser.m_gpsWarn);
				}
				else if(itype == ID_CMDRSP)
				{
					zlog_info(g_server_cat,"recv dev cmd rsp:%s",vehParser.m_szCmdRsp);
					
					cJSON* root = cJSON_CreateObject();
					cJSON_AddStringToObject(root, "imei", vehParser.m_szIMEI);
					cJSON_AddStringToObject(root, "cmdrsp", vehParser.m_szCmdRsp);
					char* out_str = cJSON_Print(root);
					

					zmq_msg_t msg;
					
					int rc = zmq_msg_init_size (&msg, strlen(out_str));
					memcpy((char *)zmq_msg_data(&msg),out_str,strlen(out_str));
					rc = zmq_sendmsg(m_pRspSender,&msg,ZMQ_NOBLOCK);
					
					cJSON_Delete(root);
				}
				else		
				{
					//m_pLogger->WriteLog(CLog::TraceLog,"Other Parse Result Thid=%ld,ParseRet=%d MoveSize=%d",m_pContext->lThid,itype,iMoveSize);
					 
				}
				iMoveSize = 0;
				itype = ID_ERR;	
				iRet =0;
			}

			if(iRet != 0)		
			{
				zlog_info(g_server_cat,"error occured or exception Thid=%ld DevNo=%d IP %s",m_pContext->lThid,m_DevState.ucDevSerial,m_szHostIp);
				break;
			}

	
		}
		else if( SOCKET_ERROR == iError )	
		{

				iError = errno;
				zlog_info(g_server_cat,"SOCKET Recv error");

				zlog_info(g_server_cat,"Plate WorkThread select error, Thid=%ld IP %s WASERROR=%ld",m_pContext->lThid,m_szHostIp,iError);
			iRet = -1;
			break;
		}
		else 
		{

			usleep(10*1000);

		}

	}//while !pIsExit

	close(m_socket);

	m_socket = NULL;
	zlog_info(g_server_cat,"Plate WorkThread exited! Ret=%d Thid=%ld DevNo=%d IP %s",iRet,m_pContext->lThid,m_DevState.ucDevSerial,m_szHostIp);
	return iRet;
}


int RecogizerSocket::Send2Dev( const void* lpBuf, int nBufLen )
{   
	zlog_info(g_server_cat,"send data to dev:");
	hzlog_debug(g_server_cat,lpBuf,nBufLen);
	int iRet = SOCKET_ERROR;
	int ileng = nBufLen;		
	int iPos = 0;				
	while(1)					
	{
		iRet = send(m_socket,(char *)lpBuf+iPos,ileng,0);
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

				printf("Error When Sending WASERROR=%d Thid=%ld DevNO=%d IP %s",iRet,m_pContext->lThid,m_DevState.ucDevSerial,m_szHostIp);
			return -1;
		}
	}
}





