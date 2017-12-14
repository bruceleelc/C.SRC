#include "VehicleInfoParser.h"
#include "zlog.h"
extern zlog_category_t* g_server_cat;

VehicleInfoParser::VehicleInfoParser(void)
{
	
	m_nSerial = 0;
	memset(&m_PkgHeader, 0x0, sizeof(PkgHeader));
	memset(&m_ReplyHeart, 0x0, sizeof(ReplyHeart));
	memset(m_szIMEI,0x0,sizeof(m_szIMEI));

	memset(&m_ReplyTime, 0x0, sizeof(m_ReplyTime));
	
}

VehicleInfoParser::~VehicleInfoParser(void)
{
	
}

unsigned short VehicleInfoParser::GetCrc16(unsigned char *pData,int nLength)
{
        unsigned short fcs = 0xffff; 

        while (nLength > 0)
        {
                fcs = (fcs >> 8) ^crctab16[(fcs ^ *pData) & 0xff];
                nLength--;
                *pData++;
        }
        return (unsigned short)(~fcs);
}




bool VehicleInfoParser::SetRelpyData(unsigned char nDataType)
{
	
	
	m_ReplyHeart.nStart = htons(0x7878);
	m_ReplyHeart.nLen = 5;
	m_ReplyHeart.nDataType = nDataType;
	m_ReplyHeart.nSerial = htons(m_nSerial);
	//m_ReplyHeart.nSerial = 0x01;
	uint8_t pData[5] = {0};
	memcpy(pData,&m_ReplyHeart.nLen,1);
	memcpy(pData+1,&m_ReplyHeart.nDataType,1);
	memcpy(pData+2,&m_ReplyHeart.nSerial,2);
	hzlog_debug(g_server_cat,pData,4);
	m_ReplyHeart.nCrc = htons(GetCrc16(pData,4));
	m_ReplyHeart.nStop = htons(0x0D0A);
	m_nSerial++;
	return true;
}

bool VehicleInfoParser::SetCMDData(char *cmd,int len,unsigned char *sndData)
{
	short tmp = 0;
	char temp = 0;
	int templ = 0;
	tmp = htons(0x7878);
	
	memcpy(sndData,&tmp,2);
	temp = 12+len;
	memcpy(sndData+2,&temp,1);
	temp = 0x80;
	memcpy(sndData+3,&temp,1);
	temp = 4+len;
	memcpy(sndData+4,&temp,1);
	templ = htonl(m_nSerial);
	memcpy(sndData+5,&templ,4);
	memcpy(sndData+9,cmd,len);
	tmp = htons(0x01);
	memcpy(sndData+9+len,&tmp,2);
	tmp = htons(m_nSerial);
	memcpy(sndData+11+len,&tmp,2);
	short nCrc = htons(GetCrc16(sndData+2,11+len));
	memcpy(sndData+13+len,&nCrc,2);
	tmp = htons(0x0D0A);
	memcpy(sndData+15+len,&tmp,2);


}
bool VehicleInfoParser::SetRelpyTimeData(unsigned char nDataType)
{
	zlog_debug(g_server_cat,"send check time msg to dev ,m_nSerial = %d",m_nSerial);
	time_t tt = time(NULL);
	tm* t= localtime(&tt);
	m_ReplyTime.nStart = htons(0x7878);
	m_ReplyTime.nLen = 11;
	m_ReplyTime.nDataType = nDataType;
	m_ReplyTime.nYear = t->tm_year -100;
	m_ReplyTime.nMon = t->tm_mon + 1;
	m_ReplyTime.nDay = t->tm_mday;
	m_ReplyTime.nHour = t->tm_hour;
	m_ReplyTime.nMin = t->tm_min;
	m_ReplyTime.nSec = t->tm_sec;
	m_ReplyTime.nSerial = htons(m_nSerial);
	//m_ReplyHeart.nSerial = htons(0x05);
	uint8_t pData[11] = {0};
	memcpy(pData,&m_ReplyTime.nLen,1);
	memcpy(pData+1,&m_ReplyTime.nDataType,1);
	memcpy(pData+2,&m_ReplyTime.nYear,1);
	memcpy(pData+3,&m_ReplyTime.nMon,1);
	memcpy(pData+4,&m_ReplyTime.nDay,1);
	memcpy(pData+5,&m_ReplyTime.nHour,1);
	memcpy(pData+6,&m_ReplyTime.nMin,1);
	memcpy(pData+7,&m_ReplyTime.nSec,1);
	memcpy(pData+8,&m_ReplyTime.nSerial,2);
	hzlog_debug(g_server_cat,pData,10);
	m_ReplyTime.nCrc = htons(GetCrc16(pData,10));
	m_ReplyTime.nStop = htons(0x0D0A);
	m_nSerial++;
	return true;
}
void VehicleInfoParser::hex_to_str(char *ptr,unsigned char *buf,int len)  
{  
    for(int i = 0; i < len; i++)  
    {  
        sprintf(ptr, "%02x",buf[i]);  
        ptr += 2;  
    }  
} 

bool VehicleInfoParser::ParserData( unsigned char* lpData , int nPkgLen, int *nResultType , int *nUsedIndex)
{
	zlog_info(g_server_cat,"ParserData data ........nPkgLen = %d.............",nPkgLen);
	hzlog_debug(g_server_cat,lpData,nPkgLen);
	if ( nPkgLen < 2) 
	{
		zlog_info(g_server_cat,"nPkgLen < 2");
		return false;
	}
	*nResultType = ID_ERR;

	unsigned char year;
	unsigned char mon;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
	

	unsigned short start = 0;
	memcpy(&start,lpData,2);
	start = ntohs(start);
	zlog_info(g_server_cat,"start :");
	hzlog_debug(g_server_cat,&start,2);
	if(0x7878 == start)
	{
		zlog_info(g_server_cat,"nPkgLen = %d",nPkgLen);
		if ( nPkgLen < sizeof(PkgHeader))
		{
			zlog_info(g_server_cat,"nPkgLen < sizeof(PkgHeader)");
			return false;
		}
		memcpy(&m_PkgHeader, lpData, sizeof(PkgHeader)); 
		zlog_info(g_server_cat,"m_PkgHeader.nLen = %d",m_PkgHeader.nLen);
		
		
		if ( m_PkgHeader.nLen+5 > nPkgLen ) 
		{
			zlog_info(g_server_cat,"m_PkgHeader.nLen > nPkgLen");
			return false;
		}	
		*nUsedIndex = m_PkgHeader.nLen+5;

		hzlog_debug(g_server_cat,lpData+2,m_PkgHeader.nLen-1);
		unsigned short tmp= htons(GetCrc16(lpData+2,m_PkgHeader.nLen-1));
		
		unsigned short nCRC;
		memcpy(&nCRC,lpData+(m_PkgHeader.nLen+1),2);
		
		if(tmp == nCRC)
		{
			zlog_info(g_server_cat,"CRC check success!");
		}
		else
		{
			zlog_info(g_server_cat,"CRC check failed!");
			zlog_info(g_server_cat,"CRC calcu value1:");
			hzlog_debug(g_server_cat,&tmp,2);
			zlog_info(g_server_cat,"CRC src value:");
			hzlog_debug(g_server_cat,&nCRC,2);
			return true;
		}
		int tmpLatitude = 0;
		int tmpLongitude = 0;
		int Direction = 0;
		short temp = 0;
		short ttt = 0;
		char  stat = 0;
		unsigned char speed = 0;
		time_t tt;
		tm* t = localtime(&tt);
		if(0x01 != m_PkgHeader.nDataType && 0 == strlen(m_szIMEI))
		{
			zlog_warn(g_server_cat,"The device is not logged in.............");
			return true;
		}
	
		switch( m_PkgHeader.nDataType )
		{
		case 0x01: 
			
			*nResultType = ID_LOGIN;
			hex_to_str(m_szIMEI,lpData+4,8);
			if('0' == m_szIMEI[0])
			{
				memmove(m_szIMEI,&m_szIMEI[1],19);
			}
			zlog_info(g_server_cat,"IMEI:%s",m_szIMEI);
			break;
		case 0x13:
			zlog_info(g_server_cat,"recv heart beat msg[%s] ==========================================",m_szIMEI);
			*nResultType = ID_HEART;
			memset(&m_devStat,0x0,sizeof(m_devStat));
			memcpy(m_devStat.m_strDeviceID,m_szIMEI, strlen(m_szIMEI));
			SYSTEMTIME stCurrent;
			GetLocalTime(&stCurrent);
	
			char tmp[50]; 
			memset(tmp,0x0,50);
			sprintf(m_devStat.m_strDeviceDate,"%04d-%02d-%02d %02d:%02d:%02d",stCurrent.wYear,stCurrent.wMonth,
				stCurrent.wDay,stCurrent.wHour,
				stCurrent.wMinute,stCurrent.wSecond);
			memcpy(&stat,lpData+4,1);
			m_devStat.m_nProtectstat = ((stat & 0x01) == 0x01?1:0);
			m_devStat.m_nAccstat = ((stat & 0x02) == 0x02?1:0);
			m_devStat.m_nChargestat = ((stat & 0x04) == 0x04?1:0);
			m_devStat.m_nGpsstat = ((stat & 0x40) == 0x40?1:0);
			m_devStat.m_nOilelecstat = ((stat & 0x80) == 0x80?1:0);
			memcpy(&m_devStat.m_cVoltagelevel,lpData+5,1);
			memcpy(&m_devStat.m_cSignalstrength,lpData+6,1);
			break;
		case 0x8A: 
			*nResultType = ID_TIME;
			break;
		case 0x22:
			*nResultType = ID_GPS;
			memset(&m_gpsData,0x0,sizeof(m_gpsData));
	
			memcpy(m_gpsData.m_strDeviceID,m_szIMEI,strlen(m_szIMEI));
			
			memcpy(&year,lpData+4,1);
			memcpy(&mon,lpData+5,1);
			memcpy(&day,lpData+6,1);
			memcpy(&hour,lpData+7,1);
			memcpy(&min,lpData+8,1);
			memcpy(&sec,lpData+9,1);
			zlog_debug(g_server_cat,"hour = %d",hour);
			t->tm_year = year+100;
			t->tm_mon = mon -1;
			t->tm_mday = day;
			t->tm_hour = hour;
			t->tm_min = min;
			t->tm_sec = sec;
			tt = mktime(t);
			t= localtime(&tt);
			zlog_debug(g_server_cat,"t->tm_hour = %d",t->tm_hour);
			tt = tt+8*3600;
			t= localtime(&tt);
			
			sprintf(m_gpsData.m_strDeviceDate,"%04d-%02d-%02d %02d:%02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
			
			zlog_info(g_server_cat,"IMEI:%s,m_gpsData.m_strDeviceDate:%s",m_szIMEI,m_gpsData.m_strDeviceDate);
			
			memcpy(&tmpLatitude,lpData+11,4);
			hzlog_debug(g_server_cat,&tmpLatitude,4);
			zlog_debug(g_server_cat,"tmpLatitude = %d",tmpLatitude);
	
			tmpLatitude = ntohl(tmpLatitude);
			zlog_debug(g_server_cat,"tmpLatitude1 = %d",tmpLatitude);
			m_gpsData.m_dfLatitude = (double)tmpLatitude/1800000;
			zlog_debug(g_server_cat,"m_dfLatitude:%lf",tmpLatitude,m_gpsData.m_dfLatitude);
			
			memcpy(&tmpLongitude,lpData+15,4);
			hzlog_debug(g_server_cat,&tmpLongitude,4);
			zlog_debug(g_server_cat,"tmpLongitude = %d",tmpLongitude);
			tmpLongitude = ntohl(tmpLongitude);
			zlog_debug(g_server_cat,"tmpLongitude1 = %d",tmpLongitude);
			m_gpsData.m_dfLongitude = (double)tmpLongitude/1800000;
			zlog_debug(g_server_cat,"m_dfLongitude:%lf",m_gpsData.m_dfLongitude);
			
			memcpy(&speed,lpData+19,1);
			m_gpsData.m_nSpeed = speed;
			zlog_debug(g_server_cat,"m_nSpeed:%d",m_gpsData.m_nSpeed);
			
			memcpy(&temp,lpData+20,2);
			temp = ntohs(temp);
			zlog_debug(g_server_cat,"temp：%d",temp);
			hzlog_debug(g_server_cat,&temp,2);
			Direction = temp & 0x3FF;
			zlog_debug(g_server_cat,"Direction = %d",Direction);
			m_gpsData.m_nDirection = Direction;
			zlog_debug(g_server_cat,"m_nDirection = %d",m_gpsData.m_nDirection);
			hzlog_debug(g_server_cat,&temp,2);
			ttt = temp & 0x400;
			zlog_debug(g_server_cat,"gggggggggg:%d",ttt);
			if (0x400 == (temp & 0x400))
			{
				m_gpsData.m_strD[0] = 'N';
			}
			else
			{
				m_gpsData.m_strD[0] = 'S';
			}
			hzlog_debug(g_server_cat,&temp,2);
			ttt = temp & 0x800;
			zlog_debug(g_server_cat,"hhhhhhhhhhh:%d",ttt);
			if (0x800 == (temp & 0x800))
			{
				m_gpsData.m_strG[0] = 'W';
			}
			else
			{
				m_gpsData.m_strG[0] = 'E';
			}
			hzlog_debug(g_server_cat,&temp,2);
			zlog_debug(g_server_cat,"ssss:%d",temp & 0x1000);
			if (0x1000 == (temp & 0x1000))
			{
				m_gpsData.m_strS[0] = 'A';
			}
			else
			{
				m_gpsData.m_strS[0] = 'V';
			}
			zlog_info(g_server_cat,"m_strD = %s,m_strG = %s,m_strS = %s",m_gpsData.m_strD,m_gpsData.m_strG,m_gpsData.m_strS);
			break;
		case 0x26:
			*nResultType = ID_WARN;
			memset(&m_gpsWarn,0x0,sizeof(m_gpsWarn));
	
			memcpy(m_gpsWarn.m_strDeviceID,m_szIMEI,strlen(m_szIMEI));
			memcpy(&year,lpData+4,1);
			memcpy(&mon,lpData+5,1);
			memcpy(&day,lpData+6,1);
			memcpy(&hour,lpData+7,1);
			memcpy(&min,lpData+8,1);
			memcpy(&sec,lpData+9,1);
			zlog_debug(g_server_cat,"hour = %d",hour);
			t->tm_year = year+100;
			t->tm_mon = mon -1;
			t->tm_mday = day;
			t->tm_hour = hour;
			t->tm_min = min;
			t->tm_sec = sec;
			tt = mktime(t);
			t= localtime(&tt);
			zlog_debug(g_server_cat,"t->tm_hour = %d",t->tm_hour);
			tt = tt+8*3600;
			t= localtime(&tt);
			sprintf(m_gpsWarn.m_strDeviceDate,"%04d-%02d-%02d %02d:%02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
			
			zlog_info(g_server_cat,"IMEI:%s,m_gpsWarn.m_strDeviceDate:%s",m_szIMEI,m_gpsWarn.m_strDeviceDate);
			
			memcpy(&tmpLatitude,lpData+11,4);
			hzlog_debug(g_server_cat,&tmpLatitude,4);
			zlog_debug(g_server_cat,"tmpLatitude = %d",tmpLatitude);
	
			tmpLatitude = ntohl(tmpLatitude);
			zlog_debug(g_server_cat,"tmpLatitude1 = %d",tmpLatitude);
			m_gpsWarn.m_dfLatitude = (double)tmpLatitude/1800000;
			zlog_debug(g_server_cat,"m_dfLatitude:%lf",tmpLatitude,m_gpsWarn.m_dfLatitude);
			
			memcpy(&tmpLongitude,lpData+15,4);
			hzlog_debug(g_server_cat,&tmpLongitude,4);
			zlog_debug(g_server_cat,"tmpLongitude = %d",tmpLongitude);
			tmpLongitude = ntohl(tmpLongitude);
			zlog_debug(g_server_cat,"tmpLongitude1 = %d",tmpLongitude);
			m_gpsWarn.m_dfLongitude = (double)tmpLongitude/1800000;
			zlog_debug(g_server_cat,"m_dfLongitude:%lf",m_gpsWarn.m_dfLongitude);
			
			memcpy(&speed,lpData+19,1);
			m_gpsWarn.m_nSpeed = speed;
			zlog_debug(g_server_cat,"m_nSpeed:%d",m_gpsWarn.m_nSpeed);
			
			memcpy(&temp,lpData+20,2);
			temp = ntohs(temp);
			zlog_debug(g_server_cat,"temp：%d",temp);
			hzlog_debug(g_server_cat,&temp,2);
			Direction = temp & 0x3FF;
			zlog_debug(g_server_cat,"Direction = %d",Direction);
			m_gpsWarn.m_nDirection = Direction;
			zlog_debug(g_server_cat,"m_nDirection = %d",m_gpsWarn.m_nDirection);
			hzlog_debug(g_server_cat,&temp,2);
			ttt = temp & 0x400;
			zlog_debug(g_server_cat,"gggggggggg:%d",ttt);
			if (0x400 == (temp & 0x400))
			{
				m_gpsWarn.m_strD[0] = 'N';
			}
			else
			{
				m_gpsWarn.m_strD[0] = 'S';
			}
			hzlog_debug(g_server_cat,&temp,2);
			ttt = temp & 0x800;
			zlog_debug(g_server_cat,"hhhhhhhhhhh:%d",ttt);
			if (0x800 == (temp & 0x800))
			{
				m_gpsWarn.m_strG[0] = 'W';
			}
			else
			{
				m_gpsWarn.m_strG[0] = 'E';
			}
			hzlog_debug(g_server_cat,&temp,2);
			zlog_debug(g_server_cat,"ssss:%d",temp & 0x1000);
			if (0x1000 == (temp & 0x1000))
			{
				m_gpsWarn.m_strS[0] = 'A';
			}
			else
			{
				m_gpsWarn.m_strS[0] = 'V';
			}
			zlog_info(g_server_cat,"m_strD = %s,m_strG = %s,m_strS = %s",m_gpsWarn.m_strD,m_gpsData.m_strG,m_gpsData.m_strS);
			
			
			
			memcpy(&stat,lpData+31,1);
			m_gpsWarn.m_nProtectstat = ((stat & 0x01) == 0x01?1:0);
			m_gpsWarn.m_nAccstat = ((stat & 0x02) == 0x02?1:0);
			m_gpsWarn.m_nChargestat = ((stat & 0x04) == 0x04?1:0);
			m_gpsWarn.m_nGpsstat = ((stat & 0x40) == 0x40?1:0);
			m_gpsWarn.m_nOilelecstat = ((stat & 0x80) == 0x80?1:0);
			memcpy(&m_gpsWarn.m_cVoltagelevel,lpData+32,1);
			memcpy(&m_gpsWarn.m_cSignalstrength,lpData+33,1);
			memcpy(&m_gpsWarn.m_cWarn,lpData+34,1);
			break;

		default: 
			return true;
		}
	}
	else if(0x7979 == start)
	{
		zlog_info(g_server_cat,"nPkgLen = %d",nPkgLen);
		if ( nPkgLen < sizeof(PkgHeader1))
		{
			zlog_info(g_server_cat,"nPkgLen < sizeof(PkgHeader1)");
			return false;
		}
		memset(&m_PkgHeader1,0x0,sizeof(PkgHeader1));
		memcpy(&m_PkgHeader1, lpData, sizeof(PkgHeader1)); 
		m_PkgHeader1.nLen = ntohs(m_PkgHeader1.nLen);
		zlog_info(g_server_cat,"m_PkgHeader1.nLen = %d",m_PkgHeader1.nLen);
		
		
		if ( m_PkgHeader1.nLen+6 > nPkgLen )
		{
			zlog_info(g_server_cat,"m_PkgHeader1.nLen > nPkgLen");
			return false;
		}	
		*nUsedIndex = m_PkgHeader1.nLen+6;
		hzlog_debug(g_server_cat,lpData+2,m_PkgHeader1.nLen);
		unsigned short tmp= htons(GetCrc16(lpData+2,m_PkgHeader1.nLen));
		
		unsigned short nCRC;
		memcpy(&nCRC,lpData+(m_PkgHeader1.nLen+2),2);
		
		if(tmp == nCRC)
		{
			zlog_info(g_server_cat,"CRC check success!");
		}
		else
		{
			zlog_info(g_server_cat,"CRC check failed!");
			zlog_info(g_server_cat,"CRC calcu value1:");
			hzlog_debug(g_server_cat,&tmp,2);
			zlog_info(g_server_cat,"CRC src value:");
			hzlog_debug(g_server_cat,&nCRC,2);
			return true;
		}
		int tmpLatitude = 0;
		int tmpLongitude = 0;
		int Direction = 0;
		short temp = 0;
		short ttt = 0;
		char  stat = 0;
		unsigned char speed = 0;
		time_t tt;
		tm* t = localtime(&tt);
		if( 0 == strlen(m_szIMEI))
		{
			zlog_warn(g_server_cat,"The device is not logged in.............");
			return true;
		}
	
		switch( m_PkgHeader1.nDataType )
		{
		case 0x21: 
			
			*nResultType = ID_CMDRSP;
			memset(m_szCmdRsp,0x0,1024);
			memcpy(m_szCmdRsp,lpData+10,m_PkgHeader1.nLen-10);
			break;
		
		default: 
			return true;
		}
	}
	else
	{
		*nUsedIndex = 1;
		return true;
	}
	
	
	return true;

}


