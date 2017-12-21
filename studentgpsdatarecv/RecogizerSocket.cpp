
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

char RecogizerSocket::check(char *src,int len)
{
	char tmp = 0;
	for(int i=0;i<len;i++)
	{
		tmp = tmp^src[i];
	}
	return tmp;
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

bool RecogizerSocket::MsgEscape(char *srcData,int len,char *destData,unsigned int &flag ,int mode)
{
	//unsigned int flag = 0;

	while(1)
	{
		if (0 == mode)
		{
			char * wellno = (char *)memchr(srcData, ESCAPE_T, len);
			if (wellno)
			{
				memcpy(destData+flag,srcData,wellno-srcData);
				flag+= wellno-srcData;
				len-=(wellno-srcData);
				srcData=wellno;

				char ch;

				if (srcData[1] == ESCAPE_2)
				{
					ch = ESCAPE;
					memcpy(destData+flag,&ch,1);
					flag+=1;
					srcData=srcData+2;
					len -= 2;
				}
				else if (srcData[1] == ESCAPE_1)
				{
					ch = ESCAPE_T;
					memcpy(destData+flag,&ch,1);
					flag+=1;
					srcData=srcData+2;
					len -= 2;
				}
				else
				{
					zlog_error(g_server_cat,"msg data error");
					return false;
					
				}
				
			}

			
			

			if (NULL == wellno)
			{
				if (len > 0)
				{
					memcpy(destData+flag,srcData,len);
					flag+=len;
				}
				break;
			}
		}
		else
		{
			
			char *wellno2 = (char *)memchr(srcData, ESCAPE, len);
			if (wellno2)
			{
				memcpy(destData+flag,srcData,wellno2-srcData);
				flag+= wellno2-srcData;
				len-=(wellno2-srcData+1);
				srcData=wellno2+1;
				WORD ch;
				ch = 0x7d02;
				memcpy(destData+flag,&ch,2);
				flag +=2;
				
			}

			char *wellno3 = (char *)memchr(srcData, ESCAPE_T, len);
			if (wellno3)
			{
				memcpy(destData+flag,srcData,wellno3-srcData);
				flag+= wellno3-srcData;
				len-=(wellno3-srcData+1);
				srcData=wellno3+1;
				WORD ch;
				ch = 0x7d01;
				memcpy(destData+flag,&ch,2);
				
				flag +=2;
				
			}

			if (NULL == wellno2 && NULL == wellno3)
			{
				if (len > 0)
				{
					memcpy(destData+flag,srcData,len);
					flag+=len;
				}
				break;
			}
			
		}
	}
	return true;
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
	header.wDevType = DEV_Q005_024_01;
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
		//zlog_debug(g_server_cat,"=============================");
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
		//zlog_debug(g_server_cat,"---------------------------------");
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

			zlog_info(g_server_cat,"recv data：%s",m_pRecvBuff);
			
			zlog_info(g_server_cat,"recv buf data ........m_ulRcvIndex = %d.............",m_ulRcvIndex);
			hzlog_debug(g_server_cat,m_pRecvBuff,m_ulRcvIndex);
			iMoveSize = 0;
			itype = ID_ERR;	
			iRet =0;
			int iResult = 0;
			char *recvBuf = NULL;
			int len = 0;
			unsigned int flag = 0;
			static unsigned int msgSn = 0;
			char msg[512] = {0};

			while(m_ulRcvIndex > 0)
			{
				char * wellno = (char *)memchr(m_pRecvBuff, ESCAPE, m_ulRcvIndex);
				if (wellno)
				{
					m_ulRcvIndex -= wellno-m_pRecvBuff+1;
					memmove(m_pRecvBuff, &m_pRecvBuff[wellno-m_pRecvBuff+1], m_ulRcvIndex);
					
					zlog_debug(g_server_cat,"m_ulRcvIndex = %d",m_ulRcvIndex);
					hzlog_debug(g_server_cat,m_pRecvBuff,m_ulRcvIndex);
					wellno = (char *)memchr(m_pRecvBuff, ESCAPE, m_ulRcvIndex);
					if (wellno)
					{
						if (recvBuf != NULL)
						{
							delete []recvBuf;
							recvBuf = NULL;
						}
						recvBuf  = new char[wellno-m_pRecvBuff+1];
						memset(recvBuf ,0x0,wellno-m_pRecvBuff+1);
						memcpy(recvBuf,m_pRecvBuff,wellno-m_pRecvBuff);
						len = wellno-m_pRecvBuff;
						zlog_debug(g_server_cat,"len = %d",len);
						
						memmove(m_pRecvBuff, &m_pRecvBuff[len+1], m_ulRcvIndex);

						m_ulRcvIndex -= len+1;
					}
					else
					{
						break;
					}
				}
				else
				{
					memset(m_pRecvBuff,0x0,RCVBUF_LENGTH);
					m_ulRcvIndex = 0;
					break;
				}
			}

			if(0 == len)
			{
				continue;
			}
			char *recvRealMsg = new char[len +1];
			char *tmpMsg = new char[len +1];
			memset(recvRealMsg,0x0,len+1);
			memset(tmpMsg,0x0,len+1);
			flag = 0;
			bool bRet = MsgEscape(recvBuf,len,recvRealMsg,flag,0);
			if(!bRet)
			{
				continue;
			}

			memcpy(tmpMsg,recvRealMsg,len);
			zlog_debug(g_server_cat,"recvRealMsg[%d]:",flag);
			hzlog_debug(g_server_cat,recvRealMsg,flag);
			vector<string> vec = split(tmpMsg,',');
			

			int vecsize = vec.size();
			if(vecsize <= 0)
			{
				continue;
			}
			zlog_debug(g_server_cat,"vecsize=%d",vecsize);
			char code[10] = {0};

			int sendlen = flag-vec[vecsize-1].size()-3-vec[0].size()-vec[1].size();
			char tmp = check(recvRealMsg,flag-strlen(vec[vecsize-1].c_str()));
			//recvRealMsg[sendlen] = 0x0;
			sprintf(code,"%d",tmp);
			zlog_debug(g_server_cat,"recvRealMsg222[%d]:",flag);
			hzlog_debug(g_server_cat,recvRealMsg,flag);
			if (code == vec[vecsize-1])
			{
				zlog_warn(g_server_cat,"code check error:code = %s,src code = %s",code,vec[vecsize-1].c_str());
			}
			else
			{
				zlog_info(g_server_cat,"code check success,code = %s!",code);
				
				if ("258" == vec[0])
				{
					if(3 > vecsize)
					{
						zlog_error(g_server_cat,"login msg error!");
						continue;
					}
					memcpy(header.szImei,vec[2].c_str(),vec[2].size());
					SYSTEMTIME stCurrent;
					GetLocalTime(&stCurrent);
					char temp[15] = {0};
					sprintf(temp,"%04d%02d%02d%02d%02d%02d",stCurrent.wYear,stCurrent.wMonth,
				stCurrent.wDay,stCurrent.wHour,
				stCurrent.wMinute,stCurrent.wSecond);
					zlog_info(g_server_cat,"login msg!");
					char tmp[256] = "33026,0,0,+8,";
					strcat(tmp,temp);
					strcat(tmp,",0,,,0,,");
					char code = check(tmp,strlen(tmp));
					memset(temp,0x0,15);
					sprintf(temp,"%d",code);
					strcat(tmp,temp);
					unsigned int flag1 = 0;
					MsgEscape(tmp,strlen(tmp),msg+1,flag1,1);
					msg[0]=0x7e;
					msg[flag1+1]=0x7e;
					zlog_info(g_server_cat,"send login rsp:%s",msg);
					hzlog_debug(g_server_cat,msg,flag1+2);
					
					Send2Dev(msg,flag1+2);
				}
				else
				{
					if("512" == vec[0])
					{
						zlog_debug(g_server_cat,"recvRealMsg333[%d]:",flag);
						hzlog_debug(g_server_cat,recvRealMsg,flag);
						header.wMsgType = DATA_STU_GPS_LOCATION;
						
						zmq_msg_t msg;
						
						int rc = zmq_msg_init_size (&msg, sizeof(header)+strlen(header.szImei)+1+sendlen);
						memcpy((char *)zmq_msg_data(&msg),&header,sizeof(header));
						memcpy((char *)zmq_msg_data(&msg)+sizeof(header),header.szImei,strlen(header.szImei));
						memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei),",",1);
						memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei)+1,recvRealMsg+vec[0].size()+vec[1].size()+2,sendlen);
						zlog_info(g_server_cat,"send msg to MsgDealThread:");
						hzlog_debug(g_server_cat,(char *)zmq_msg_data(&msg),sizeof(header)+strlen(header.szImei)+1+sendlen);
						zlog_debug(g_server_cat,"recvRealMsg1111111:");
						hzlog_debug(g_server_cat,recvRealMsg+vec[0].size()+vec[1].size()+2,sendlen);
						rc = zmq_sendmsg(m_pGpsSender,&msg,ZMQ_NOBLOCK);
					}
					else if("514" == vec[0])
					{
						header.wMsgType = DATA_STU_GPS_LOCATION;
						int msgNum = atoi(vec[2].c_str());
						for (int m = 0,j=3,k=3;m<msgNum,j<vec.size(),k<vec.size();m++,j++)
						{
							j+=6;
							if("1" == vec[j])
							{
								j+=5;
								char *msgTmp = new char[sendlen];
								memset(msgTmp,0x0,sendlen);
								int i=0;
								for(;k<=j;k++)
								{
									if("," == vec[k])
									{
										continue;
									}
									memcpy(msgTmp+i,vec[k].c_str(),vec[k].size());
									i+=strlen(vec[k].c_str());
									msgTmp[i+1] = ',';
									i++;
								}
								msgTmp[i] = 0x0;
								zmq_msg_t msg;
						
								int rc = zmq_msg_init_size (&msg, sizeof(header)+strlen(header.szImei)+1+strlen(msgTmp));
								memcpy((char *)zmq_msg_data(&msg),&header,sizeof(header));
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header),header.szImei,strlen(header.szImei));
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei),",",1);
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei)+1,msgTmp,strlen(msgTmp));
								rc = zmq_sendmsg(m_pGpsSender,&msg,ZMQ_NOBLOCK);
								delete []msgTmp;
							}
							else if("2" == vec[j])
							{
								j+=6;
								char *msgTmp = new char[sendlen];
								memset(msgTmp,0x0,sendlen);
								int i=0;
								for(;k<=j;k++)
								{
									if("," == vec[k])
									{
										continue;
									}
									memcpy(msgTmp+i,vec[k].c_str(),vec[k].size());
									i+=strlen(vec[k].c_str());
									msgTmp[i+1] = ',';
									i++;
								}
								msgTmp[i] = 0x0;
								zmq_msg_t msg;
						
								int rc = zmq_msg_init_size (&msg, sizeof(header)+strlen(header.szImei)+1+strlen(msgTmp));
								memcpy((char *)zmq_msg_data(&msg),&header,sizeof(header));
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header),header.szImei,strlen(header.szImei));
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei),",",1);
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei)+1,msgTmp,strlen(msgTmp));
								rc = zmq_sendmsg(m_pGpsSender,&msg,ZMQ_NOBLOCK);
								delete []msgTmp;
							}
							else if("3" == vec[j])
							{
								j+=2;
								if("0" == vec[j])
								{
									j+=6;
								}
								else
								{
									j+=7;
								}
								char *msgTmp = new char[sendlen];
								memset(msgTmp,0x0,sendlen);
								int i=0;
								for(;k<=j;k++)
								{
									if("," == vec[k])
									{
										continue;
									}
									memcpy(msgTmp+i,vec[k].c_str(),vec[k].size());
									i+=strlen(vec[k].c_str());
									msgTmp[i+1] = ',';
									i++;
								}
								msgTmp[i] = 0x0;
								zmq_msg_t msg;
						
								int rc = zmq_msg_init_size (&msg, sizeof(header)+strlen(header.szImei)+1+strlen(msgTmp));
								memcpy((char *)zmq_msg_data(&msg),&header,sizeof(header));
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header),header.szImei,strlen(header.szImei));
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei),",",1);
								memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei)+1,msgTmp,strlen(msgTmp));
								rc = zmq_sendmsg(m_pGpsSender,&msg,ZMQ_NOBLOCK);
								delete []msgTmp;
							}
						}
						
						
					}
					else if("2" == vec[0])
					{
						header.wMsgType = DATA_STU_DEV_STATUS;
						zmq_msg_t msg;
						
						int rc = zmq_msg_init_size (&msg, sizeof(header)+strlen(header.szImei)+1+sendlen);
						memcpy((char *)zmq_msg_data(&msg),&header,sizeof(header));
						memcpy((char *)zmq_msg_data(&msg)+sizeof(header),header.szImei,strlen(header.szImei));
						memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei),",",1);
						memcpy((char *)zmq_msg_data(&msg)+sizeof(header)+strlen(header.szImei)+1,recvRealMsg+vec[0].size()+vec[1].size()+2,sendlen);
						rc = zmq_sendmsg(m_pGpsSender,&msg,ZMQ_NOBLOCK);
					}
					
				}
				
			}

			delete []recvRealMsg;
			delete []tmpMsg;

	
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





