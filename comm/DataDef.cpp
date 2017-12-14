/*
 * DataDef.cpp
 *
 *  Created on: 2011-10-12
 *      Author: root
 */
#ifdef linux
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "DataDef.h"


//SYSTEMTIME 为windows下专用, linux时使用以下函数替换GetLocalTime
//************************************************************
//	函数:	GetLocalTime
//	功能:	linux版的GetLocalTime获取本地时间
//	访问权:	public
//	参数:	SYSTEMTIME* lpSystemTime	SYSTEMTIME指针
//	返回值:	无
//************************************************************
void GetLocalTime(SYSTEMTIME* lpSystemTime)
{
	struct timeval tv;
	gettimeofday (&tv , NULL);

	tm tm_now ;
	localtime_r(&tv.tv_sec,&tm_now);

	lpSystemTime->wYear = tm_now.tm_year+1900;			//年份
	lpSystemTime->wMonth = tm_now.tm_mon+1;				//月 tm[0-11] sys[1-12]
	lpSystemTime->wDay = tm_now.tm_mday;						//日
	lpSystemTime->wDayOfWeek = (tm_now.tm_wday+1)%7 ;		// tm一星期的日数, 从星期一算起, 范围为0-6 sys从星期日算起
	lpSystemTime->wHour =tm_now.tm_hour;					//小时
	lpSystemTime->wMinute = tm_now.tm_min;				//分钟
	lpSystemTime->wSecond = tm_now.tm_sec;				//秒
	lpSystemTime->wMilliseconds = tv.tv_usec/1000;			//毫秒
}

void SystemTime2Time_T(LPSYSTEMTIME pSysTime,time_t* pTime)
{
	tm tm_tmp = {pSysTime->wSecond, pSysTime->wMinute, pSysTime->wHour, pSysTime->wDay, pSysTime->wMonth-1, pSysTime->wYear-1900, pSysTime->wDayOfWeek, 0, 0};
	*pTime = mktime(&tm_tmp);
}

unsigned long GetTickCount(void)
{
	struct timespec ts;
	int iVal = clock_gettime(_POSIX_MONOTONIC_CLOCK,&ts);
	if(iVal == 0)		//正常取得启动后的毫秒数
	{
		return (ts.tv_sec*1000 + ts.tv_nsec/(1000*1000));
	}
	else	//失败, 一般是系统不支持 arm?
	{
		perror("call clock_gettime fail");
		printf("error %d\n",errno);
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);;
	}

	/*  clock_gettime的CLOCK_ID 使用值的说明
	    On  POSIX  systems  on  which these functions are available, the symbol
       _POSIX_TIMERS is defined in <unistd.h> to a value greater than 0.   The
       symbols  _POSIX_MONOTONIC_CLOCK,  _POSIX_CPUTIME, _POSIX_THREAD_CPUTIME
       indicate      that      CLOCK_MONOTONIC,      CLOCK_PROCESS_CPUTIME_ID,
       CLOCK_THREAD_CPUTIME_ID are available.  (See also sysconf(3).)
	 */
}
#else
#include "StdAfx.h"

#endif
