
#ifndef RECOGIZERLISTENER_H_
#define RECOGIZERLISTENER_H_
#pragma once
#include <vector>

#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "zmq.hpp"
#include <fcntl.h>

#include <pthread.h>
#include <signal.h>		


#define SOCKET_ERROR -1
typedef int	SOCKET;

typedef pthread_t HANDLE;



#define RET_CLIENT_CONNECTED   1	
#define RET_LISTEN_TIMEOUT	   2	
#define RET_ERROR			   -1	
#define RET_SUCCESS			   0	


typedef void * ( * WORKTHREAD)(void *pContext);


typedef struct tagThreadContext
{
	unsigned long		lThid;		
	void*				pZCntext;   
	SOCKET				socket;		
	unsigned long		ulIP;		
	unsigned short		usPort;		
	HANDLE				handle;		
	//int				iState;		
}ThreadContext;


typedef std::vector<ThreadContext*> ThreadList;
typedef std::vector<ThreadContext*>::iterator ThreadListIterator;

class RecogizerListener
{
public:
	RecogizerListener(void);
	~RecogizerListener(void);
public:	

	void Initial(unsigned int iPort,void *pContext,WORKTHREAD pThread);


	bool CreateBinding(void);

	int	 ListenAndAcc(void);

	int	 CheckThread(void);
	
	int  EndWork(void);
	
	unsigned long	GetNextThId(void);
public:
	void*		m_pZCntext;			//zmq上下文
private:
	
	unsigned long		m_lThreadId;		
	WORKTHREAD			m_pWorkThread;		
	ThreadList			m_threadlist;		
	SOCKET				m_socket;			
	unsigned int		m_iSvrPort;			
};


#endif
