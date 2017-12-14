

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "RecogizerListener.h"
#include "zlog.h"
extern zlog_category_t* g_server_cat;
RecogizerListener::RecogizerListener(void):m_lThreadId(0),m_iSvrPort(0),m_pWorkThread(NULL)
{
	m_socket = NULL;

}


RecogizerListener::~RecogizerListener(void)
{
	EndWork();					
	m_pWorkThread = NULL;
}


void RecogizerListener::Initial(unsigned int iPort,void *pContext,WORKTHREAD pThread)
{
	m_iSvrPort = iPort;
	m_pWorkThread = pThread;
	m_pZCntext = pContext;
}


bool RecogizerListener::CreateBinding( void )
{

	int iError = SOCKET_ERROR;

	m_socket = NULL;
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == SOCKET_ERROR)		
	{
		zlog_info(g_server_cat,"Create Listener SOCKET Fail, %d",errno);
		
		m_socket = NULL;
		return false;
	}

	int iOpt = 1;
	unsigned int iSize = sizeof(iOpt);
	if(setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,&iOpt,iSize) == SOCKET_ERROR)
	{
		char szMsg[640];
		zlog_info(g_server_cat,"Set Listener Socket OPT Fail, %d",errno);
		perror(szMsg);
		close(m_socket);
		m_socket = NULL;
		return false;
	}

	struct sockaddr_in  localaddr;
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(m_iSvrPort);
	zlog_info(g_server_cat,"Bind Local Port, %d",m_iSvrPort);
	if (bind(m_socket, (sockaddr*)&localaddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		char szMsg[640];
		zlog_info(g_server_cat,"Bind Local Port Fail, %d",errno);
		perror(szMsg);
		close(m_socket);
		m_socket = NULL;
		return false;
	}
	

	iError = listen(m_socket,SOMAXCONN);
	if (iError == SOCKET_ERROR)
	{
		iError = errno;
		close(m_socket);
		m_socket = NULL;
		return false;
	}
	return true;
}

int RecogizerListener::ListenAndAcc( void )
{
	if ( NULL == m_pWorkThread )
	{ 
		zlog_info(g_server_cat,"listen socket unCreate or No WorkThread\n");
		return RET_ERROR;
	}

	int iError = SOCKET_ERROR;
	ThreadContext *pContext = NULL;

	sockaddr_in  addrClient;
	unsigned int len=sizeof(sockaddr_in);

	fd_set fsBind;
	FD_ZERO(&fsBind);
	FD_SET(m_socket,&fsBind);

	timeval  tv;
	tv.tv_sec = 1;			
	tv.tv_usec = 0;
	iError = select(m_socket+1,&fsBind,NULL,NULL,&tv);
	if (iError > 0)
	{

		
		SOCKET sockConn = accept(m_socket,(sockaddr*)&addrClient,&len);
		pContext = new ThreadContext;
		pContext->lThid = GetNextThId();
		pContext->pZCntext = m_pZCntext;
		pContext->socket = sockConn;
		pContext->ulIP = addrClient.sin_addr.s_addr;
		pContext->usPort = addrClient.sin_port;
		pContext->handle = NULL;

		in_addr  addr;
		addr.s_addr = pContext->ulIP;
		char * szIp = inet_ntoa(addr);
		zlog_info(g_server_cat,"DEV  Connected , Thid %d IP %s",pContext->lThid,szIp);


		iError = pthread_create (&(pContext->handle), NULL, m_pWorkThread, pContext);

		if (iError != 0)
		{
			close(pContext->socket);
			delete [] pContext;
			zlog_info(g_server_cat,"Create Plate Work Thread Fail! ");
			return RET_ERROR;
		}

		//m_threadlist.push_back(pContext);
		return RET_CLIENT_CONNECTED;


	}else if(iError == 0)
	{
		FD_ZERO(&fsBind);
		return RET_LISTEN_TIMEOUT;
	}
	else
	{

		perror("An error occurred when select,");
		printf("An error occurred when select, %d SOCKERR=%d",iError,errno);

		FD_ZERO(&fsBind);
		return RET_ERROR;
	}
}



int RecogizerListener::EndWork( void )
{
	close(m_socket);

	return RET_SUCCESS;
}

unsigned long RecogizerListener::GetNextThId( void )
{
	return ++m_lThreadId;
}



int RecogizerListener::CheckThread( void )
{
	
	return RET_SUCCESS;
}

