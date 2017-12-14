//********************************************************
//文件:Tool.h
//说明:各种辅助函数的声明
//作者:zdx
//创建时间:2011-07-28 10:00
//----------------------------------------------------------
//修改记录:
//修改者:张笃续
//修改日期:2011-08-18 10:05
//修改内容:增加除空格函数
//********************************************************

#ifndef TOOL_H_
#define TOOL_H_
#pragma once

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>  
#include <iostream> 
#include <fcntl.h>


// ***BEGIN***  PlateDataReciverV2.1 double direction DC 2014/9/1 add by angxin
#include <map> 			
#include <string>
#include <vector>	
#include <sstream>		
// ****END****  PlateDataReciverV2.1 double direction DC 2014/9/1 add by angxin

#ifdef linux
#include <unistd.h>
#include <assert.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <process.h>
#include <direct.h>					//mkdir
#include  <io.h>					//access函数支持
#endif


// ***BEGIN***  PlateDataReciverV2.1 double direction DC 2014/9/1 add by angxin
using namespace std;
// ****END****  PlateDataReciverV2.1 double direction DC 2014/9/1 add by angxin

#define PIDROOT "/var/run/"
//BOOL SUPPORT
#define FALSE 0
#define TRUE	1



#define ASSERT(e) do { if(!(e)) { printf("AssertException: " #e \
" at %s:%d\naborting..\n", __FILE__, __LINE__); abort(); } } while(0)
#define STRERROR            strerror(errno)

//**************************************************************
//	函数:	isAllNum
//	功能:	检测所有字符均为数字
//	参数:
//	- szData:待检查字符串
//	- iSize:长度
//	返回值:
//	均为数字		返回 TRUE
//	存在非数字字符	返回 FALSE
//*************************************************************
bool isAllNum(const char* szData);
//************************************************************
//	函数:	rmSpace
//	功能:	删除字符串左右空格和制表符
//	访问权:	public
//	参数:	char * str
//	返回值:	char *
//************************************************************
char *rmSpace(char *str);

//**************************************************************
//	函数:	GetConfigString
//	功能:	获取*.ini配置文件字符型配置选项
//	参数:
//	- psFile:文件名字串
//	- psItem:指定项名字串
//	- psName:指定名称
//	- szDataBuf:接收字串内容缓冲区
//	- iBufLen:缓冲区szDataBuf的长度
//	- szDefaultValue:默认值
//	返回值:
//	- 成功	返回字串内容指针
//	- 失败	返回NULL，将缓冲赋值为默认值
//	************************************************************
char* GetConfigString( const char* psFile, const char* psItem, const char* psName,char* szDataBuf,unsigned int iBufLen,const char* szDefaultValue);


//**************************************************************
//	函数:	GetConfigInt
//	功能:	获取*.ini配置文件数字型配置选项
//	参数:
//	- psFile:文件名字串
//	- psItem:指定项名字串
//	- psName:指定名称
//	- iDefaultValue:默认值
//	返回值:
//	- 成功	返回参数值
//	- 失败	返回默认值
//9**********************************************************
int GetConfigInt( const char* psFile, const char* psItem, const char* psName,const int iDefaultValue);

// 解密HEX格式的密文并转换成明文
bool XorDecodeFromHexString(const char* szSrc,char* szDes,const int iDesLeng,const char* szKey,const int iKeyLeng);

int HexstrToAscii( const char* szSrc,int iSrcLeng,char* szDest,int iDesLeng );

bool XorEncode( const char* szSrc,char* szDes,const int iStrLeng,const char* szKey,const int iKeyLeng );

/*
 * 读取文件信息到 Buffer
 * FileName 文件名称 Buffer缓冲区
 */
bool ReadFile(char* Buffer,char* FileName);
/*
 * 保存信息到文件中
 * FileName 文件名称 Buffer缓冲区
 */
bool SaveFile(char* Buffer,char* FileName);

#ifdef linux

/**
 * 检查文件是否存在
 * @file 文件路径
 * @return 如果存在返回TRUE，否则FALSE
 */
int file_exist(const char *file);
/**
 * 检查是否文件是规则的
 * @param 路径
 * @return 规则文件TRUE，否则FALSE
 *
 */
int file_isFile(const char *file);
/*
 *  获取已存在的实例的PID
 *  @pidfile 存放pid的文件的路径
 *  @prog	   程序名称
 */
pid_t tool_getPid(const char *pidfile,const char* prog);
/**
 * 是否已有实例存在
 * @pidfile pid文件路径
 * @prog 程序名称
 * @若存在，则返回原进程的PID 否则返回 FALSE
 */
int exist_instance(const char *pidfile,const char* prog);
/**
 * 是否已有实例存在
 * @szAppName 程序名称
 * @若存在，则返回原进程的PID 否则返回 FALSE
 */
int exist_instance(const char *szAppName);

/*
 * 无阻塞读按键
 * @返回值 按键值
 */
int getkey(void);

/*
 * 记录进程PID到 /var/run/[appname].pid
 * @成功，TRUE，其他FALSE
 */
int writePid(const char* szAppName);

/*
 * 发送终止信号到对应程序进程
 * @pid 进程pid
 */
int sendExitSignal(pid_t pid);

/*
 * 发送终止信号到对应程序进程，根据读取pid文件获取
 * @szAppName 进程名称
 */
int sendExitSignal(const char* szAppName);

vector<string> split(char *src,char *delimiters);
/*
 * 获取程序当前路径
 * @szBuff 路径缓冲
 * @iLeng  缓冲长度
 */
int GetCurPath(char *szBuff, int iLeng);


#endif

#endif	//TOOL_H_
