//********************************************************
//文件:Assistant.cpp 
//说明:车牌、标签需要用到的辅助类, 如生成ID号、计算过时
//作者:zdx
//创建时间:2011-08-03 09:00
//----------------------------------------------------------
//修改记录:
//修改者:
//修改日期:
//修改内容:
//********************************************************
#include "stdafx.h"
#include "Assistant.h"

//*************************************************************
//	函数:	Assistant
//	功能:	初始化成员
//	参数:	
//	- lpszFileName 配置文件路径
//	返回值:
//************************************************************
Assistant::Assistant(void):m_iSerial(0)
{
	memset(&m_stLastTime, 0, sizeof(SYSTEMTIME));
	memset(m_szProcName,0x00,sizeof(m_szProcName));
}

Assistant::~Assistant(void)
{
}

//*************************************************************
//	函数:	GenerateRecordID
//	功能:	生成记录号
//	参数:	
//  参数: char lpszRecordID[28]   待生成的ID
//  参数: LPSYSTEMTIME lpstRecord 收到标签/车牌记录的时间
//  参数: int iUnitNum			  卡口编号
//  参数: int nDirection		  方向  参见方向参数配置
//  参数: int nRoadNum			  车道号
//  参数: int nType				  记录号类型(备用) 0 标签 1车牌	   
//	返回值:无
//************************************************************
void Assistant::GenerateRecordID(char lpszRecordID[28], LPSYSTEMTIME lpstRecord, int iUnitNum, int nDirection, int nRoadNum, int nType)
{
	int nSerial = GetSerialNumber(lpstRecord, nType);	//取末2位序号
	//平台切换时, 应采用C标准时间函数

	//车牌 YYYYMMDDHHMMSSxxxxxxxxQyyzz （年月日时分秒 x-8位站号 1位车道方向编号 y-2位道号 z-2位流水号）
	//标签 YYYYMMDDHHMMSSxxxxxxxxQyyzz （年月日时分秒 x-8位站号 1位车道方向编号 y-2位道号 z-2位流水号）

	//已不使用  ---标签 YYYYMMDDHHMMSSxxxxxxxxQyyzz （年月日时分秒 x-8位站号 y-1位天线号 2位读写器ID号 z-2位流水号）
	sprintf(lpszRecordID, "%04d%02d%02d%02d%02d%02d%08d%01d%02d%02d", 
		lpstRecord->wYear, 
		lpstRecord->wMonth,
		lpstRecord->wDay,
		lpstRecord->wHour,
		lpstRecord->wMinute,
		lpstRecord->wSecond,
		iUnitNum,		// 站号
		nDirection,		// 车道方向
		nRoadNum,		// 道号或者天线号
		nSerial			// 流水号
		);
}



//*************************************************************
//	函数:	GetSerialNumber
//	功能:	生成记录号
//	参数:	
//  参数: LPSYSTEMTIME lpstRecord  记录收取时间
//	参数: int nType				   记录类型 0标签 1车牌
//	返回值:无
//************************************************************
int Assistant::GetSerialNumber( LPSYSTEMTIME lpstRecord, int nType )
{
	// 判断前后时间是否有变化
	if (lpstRecord->wSecond != m_stLastTime.wSecond ||
		lpstRecord->wMinute != m_stLastTime.wMinute)
	{
		memcpy(&m_stLastTime, lpstRecord, sizeof(SYSTEMTIME));	// 保存时间
		m_iSerial = 0;										// 前后时间有变化, 流水号清零
	}

	m_iSerial++;			// 流水号 + 1, 返回的流水号最小为 1,  最大为 99

	if (m_iSerial > 99)		// 流水号最大为99, 一般情况应该不存在
	{
		m_iSerial = 99;		// 程序不应该执行到此处, 否则将会产生相同的记录号
	}

	return m_iSerial;
}


//*************************************************************
//	函数:	MakeDevState
//	功能:	生成记录号
//	参数:	
//	参数: BYTE direction	方向
// 	参数: BYTE road			道号
// 	参数: BYTE type			设备类型
// 	参数: BYTE sequence		设备序号
// 	参数: BYTE state		状态
//	返回值:无
//************************************************************
DWORD Assistant::MakeDevState( BYTE direction, BYTE road, BYTE type, BYTE sequence, BYTE state )
{
	DWORD  dwCode = 0;
	//按位排列
	dwCode |= (direction << 30);
	dwCode |= (road << 24);
	dwCode |= (type << 16);
	dwCode |= (sequence << 8);
	dwCode |= (state);

	return dwCode;
}


//************************************************************
//	函数:    GetRoadDirection
//	功能:  通过车道编号得到车道方向 
//	参数:	int iRoadNo  车道号
//	参数: 	const DirConf pDirConf[]	车道所在方向
//	参数: 	int iSize					pDirConf数组大小
//	返回值:  int  车道方向  详见配置说明
//************************************************************
int Assistant::GetRoadDirection( int iRoadNo,const char* pDirConf[],int iSize)
{
	if ( iRoadNo > 8 || iRoadNo < 1 )
		return 1;
	int iDirection = 0;
	char* pToken;
	char* pNext_token;
	int iTmp;
	bool bStopScan = false;
	char szTmp[64];
	for (int i=0; i<iSize ; i++)		//循环从8个车道方向配置中查找对应的车道
	{
		memcpy( szTmp , pDirConf[i],64);
#ifndef linux
		pToken = strtok_s( szTmp ,",",&pNext_token);				//linux use strtok_r
#else
		pToken = strtok_r(szTmp,",",&pNext_token);
#endif
		while ( pToken != NULL )
		{
			iTmp = atoi(pToken); 
			if ( iTmp == iRoadNo )			//找到对应车道方向
			{
				iDirection = i;
				bStopScan = true;
				break;
			}
#ifndef linux
			pToken = strtok_s(NULL,",",&pNext_token); 
#else
			pToken = strtok_r(NULL,",",&pNext_token);
#endif
		}

		if ( bStopScan )
			break;
	}

	return iDirection;
}


void Assistant::SetProcName(const char *szName)
{
	sprintf(m_szProcName,szName);
}


bool Assistant::CheckInstance(void)
{
#ifdef linux
	m_pid = getpid();
#endif
	return false;
}

int Assistant::GetCurPath(char *szBuff, int iLeng)
{
	int  iRet = -1;
#ifdef linux
	iRet = readlink("/proc/self/exe" , szBuff , iLeng);
#endif
	return iRet;
}

//************************************************************
//	函数:    WritePid
//	功能:  	将程序的PID写入到文件中
//	参数:	void
//	返回值:  void
//***********************************************************
bool Assistant::WritePid(void)
{
	if(strlen(m_szProcName) <1)
	{
		return false;
	}

	char szPath[256];
	sprintf(szPath,"/var/run/%s",m_szProcName);
	return false;
}




