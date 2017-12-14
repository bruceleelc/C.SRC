//********************************************************
//文件:Assistant.h 
//说明:车牌、标签需要用到的辅助类，如生成ID号、计算过时
//作者:zdx
//创建时间:2011-08-03 09:00
//----------------------------------------------------------
//修改记录:
//修改者:
//修改日期:
//修改内容:
//********************************************************

#ifndef ASSISTANT_H_
#define	ASSISTANT_H_

#pragma once

#ifndef linux
//windows
#include <Windows.h>

#else
//linux函数库
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#endif

#include "DataDef.h"

class Assistant
{
public:
	Assistant(void);		//初始化
	~Assistant(void);

	// 产生27位的纪录号, nType = 0 : 车牌识别， nType = 1 : RFID标签
	void GenerateRecordID(char lpszRecordID[28], LPSYSTEMTIME lpstRecord, int iUnitNum, int nDirection, int nRoadNum, int nType);
	// 根据类型（nType = 0 : 车牌识别， nType = 1 : RFID标签）获取流水号，时间变化（秒）则流水号清零
	int GetSerialNumber(LPSYSTEMTIME lpstRecord, int nType);
	//生成设备状态字段
	DWORD MakeDevState(BYTE direction, BYTE road, BYTE type, BYTE sequence, BYTE state);
public:
	//通过车道编号得到车道方向
	static int GetRoadDirection( int iRoadNo,const char* pDirConf[],int iSize);
public:
	//linux下获取程序当前路径
	static int GetCurPath(char *szBuff,int iLeng);

public:
	//设置进程名称
	void SetProcName(const char *szName);
	//检查是否存在程序实例 true成功--当前只有本实实例， false 失败，已有其他实例在运行
	bool CheckInstance( void );
private:
	//将PID写入文件
	bool WritePid( void );
private:
	SYSTEMTIME m_stLastTime;		//最后一次标签或者车牌读取的时间
	int		   m_iSerial;			//记录流水号

private:
	char m_szProcName[64];		//进程名称
	unsigned long	m_pid;			//进程ID
};

#endif	/* ASSISTANT_H_ */
