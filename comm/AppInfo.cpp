#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "AppInfo.h"

AppInfo::AppInfo()
{
	m_pid = " ";					//pid 值
	m_cpu_usage = "0%";				//cpu 占用率
	m_alivetime = "0 sec";			//已运行时间
	m_memory_usage = "0";			//内存用量
	m_status = "0";				//运行状态
	m_version = "Unknow Version";
	m_allinfo = DumpProcInfo();
}

AppInfo::AppInfo( const AppInfo& obj )
{
	m_appid = obj.m_appid;						//模块ID
	m_name = obj.m_name;					//文件名称
	m_pidfile = obj.m_pidfile;				//pid文件路径
	m_startcmd = obj.m_startcmd;			//运行命令
	m_stopcmd = obj.m_stopcmd;				//终止命令

	m_pid  = obj.m_pid;					//pid 值
	m_cpu_usage = obj.m_cpu_usage;				//cpu 占用率
	m_alivetime = obj.m_alivetime;				//已运行时间
	m_memory_usage = obj.m_memory_usage;			//内存用量
	m_status = obj.m_status;				//运行状态
}

AppInfo& AppInfo::operator=( const AppInfo &obj )
{
	if (this == &obj)
	{
		return *this;
	}
	m_appid = obj.m_appid;						//模块ID
	m_name = obj.m_name;					//文件名称
	m_pidfile = obj.m_pidfile;				//pid文件路径
	m_startcmd = obj.m_startcmd;			//运行命令
	m_stopcmd = obj.m_stopcmd;				//终止命令

	m_pid  = obj.m_pid;					//pid 值
	m_cpu_usage = obj.m_cpu_usage;				//cpu 占用率
	m_alivetime = obj.m_alivetime;				//已运行时间
	m_memory_usage = obj.m_memory_usage;			//内存用量
	m_status = obj.m_status;				//运行状态
	return  *this;
}

string AppInfo::getAppid() const
{
	return m_appid;
}

string AppInfo::getName() const
{
	return m_name;
}

string AppInfo::getPidfile() const
{
	return m_pidfile;
}

string AppInfo::getStartcmd() const
{
	return m_startcmd;
}

string AppInfo::getStopcmd() const
{
	return m_stopcmd;
}

void AppInfo::setAppid( string strAppid )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	strcpy(szTmp,strAppid.c_str());
	rmSpace(szTmp);
	this->m_appid = string(szTmp);
}

void AppInfo::setName( string strName )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	strcpy(szTmp,strName.c_str());
	rmSpace(szTmp);
	this->m_name = string(szTmp);
#ifndef linux
	m_Info.setProcessNameNoQuery(this->m_name);
#endif
}

void AppInfo::setPidfile( string strPidfile )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	strcpy(szTmp,strPidfile.c_str());
	rmSpace(szTmp);
	this->m_pidfile =string(szTmp);
}

void AppInfo::setStartcmd( string strStartcmd )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	strcpy(szTmp,strStartcmd.c_str());
	rmSpace(szTmp);
	this->m_startcmd = string(szTmp);
}

void AppInfo::setStopcmd( string strStopcmd )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	strcpy(szTmp,strStopcmd.c_str());
	rmSpace(szTmp);
	this->m_stopcmd = string(szTmp);
}

string AppInfo::getStatus() const
{
	return m_status;
}

void AppInfo::setStatus( string val )
{
	m_status = val;
}

void AppInfo::setStatus( int ival )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	sprintf(szTmp,"%d",ival);
	m_status = string(szTmp);
}

string AppInfo::getMemory_usage() const
{
	return m_memory_usage;
}

void AppInfo::setMemory_usage(string val )
{
	m_memory_usage = val;
}

void AppInfo::setMemory_usageAT( int ival )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	if (ival <= 1024*1024 )		//10k
	{
		sprintf(szTmp,"%d KB",ival);
	}
	else
	{
		sprintf(szTmp,"%0.2f MB",(float)ival/(1024.00*1024.00));
	}
	m_memory_usage = string(szTmp);
}

void AppInfo::setMemory_usageKB( int ival )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	sprintf(szTmp,"%d",ival/1024);
	m_memory_usage = string(szTmp);
}

string AppInfo::getAlivetime() const
{
	return m_alivetime;
}

void AppInfo::setAlivetime( string val )
{
	m_alivetime = val;
}


/*
void AppInfo::setAlivetime( long upt )
{
	// 1000
	// 秒转换成 日时分秒 --  aa day bb hour cc min dd sec
	long t_day  = ival /(3600*24);
	long t_hour = ( ival - t_day*(3600*24)) / 3600;
	long t_min  = ( ival - t_day*(3600*24) - t_hour*3600 )/60;
	long t_sec  = ival - t_day*(3600*24) - t_hour*3600 - t_min*60 ;

	char szTmp[512];
	memset(szTmp,0x00,512);
	sprintf(szTmp,"%ld day %ld hour %ld min %ld sec ",t_day,t_hour,t_min,t_sec);
	m_alivetime = string(szTmp);
}
*/
#ifndef linux
void AppInfo::setAlivetime( long upt )
{
	static int min = 60;
	static int hour = 3600;
	static int day = 86400;
	long t_day  ;
	long t_hour ;
	long t_min ;
	long t_sec;
	char buf[512];
	char *p = buf;
	// 秒转换成 日时分秒 --  aa day bb hour cc min dd sec
	*buf = 0;
	if(upt < 0)
		sprintf(buf,"0 sec");
	else
	{
		if((t_day = upt/day)>0) {
			p += _snprintf(p, 512-(p-buf), "%ldday ", t_day);
			upt -= t_day*day;
		}

		if((t_hour = upt/hour)>0 || (t_day > 0)) {
			p += _snprintf(p, 512-(p-buf), "%ldhour ", t_hour);
			upt -= t_hour*hour;
		}
		t_min = upt/min;
		t_sec= upt - t_min*min;
		p += _snprintf(p, 512 - (p - buf), "%ldmin ", t_min);
		_snprintf(p, 512 - (p - buf), "%ldsec", t_sec);
	}
	m_alivetime = string(buf);
}

#else
void AppInfo::setAlivetime(time_t upt)
{

    static int min = 60;
    static int hour = 3600;
    static int day = 86400;
	long t_day  ;
	long t_hour ;
	long t_min ;
	long t_sec;
    char buf[512];
    char *p = buf;
	// 秒转换成 日时分秒 --  aa day bb hour cc min dd sec
    *buf = 0;
    if(upt < 0)
         sprintf(buf,"0 sec");
    else
    {
        if((t_day = upt/day)>0) {
                p += snprintf(p, 512-(p-buf), "%ldday ", t_day);
                upt -= t_day*day;
        }

        if((t_hour = upt/hour)>0 || (t_day > 0)) {
                p += snprintf(p, 512-(p-buf), "%ldhour ", t_hour);
                upt -= t_hour*hour;
        }
        t_min = upt/min;
        t_sec= upt - t_min*min;
    	p +=snprintf(p, 512 - (p - buf), "%ldmin ", t_min);
    	snprintf(p, 512 - (p - buf), "%ldsec", t_sec);
    }
    m_alivetime = string(buf);
}
#endif
string AppInfo::getCpu_usage() const
{
	return m_cpu_usage;
}

void AppInfo::setCpu_usage( string val )
{
	m_cpu_usage = val;
}

void AppInfo::setCpu_usage( int ival )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	sprintf(szTmp,"%d",ival);
	m_cpu_usage = string(szTmp);
}

string AppInfo::getPid() const
{
	return m_pid;
}

int AppInfo::getPidInt() const
{
	return atol(m_pid.c_str());
}
void AppInfo::setPid( string val )
{
	m_pid = val;
}

void AppInfo::setPid( int ival )
{
	char szTmp[512];
	memset(szTmp,0x00,512);
	sprintf(szTmp,"%d",ival);
	m_pid = string(szTmp); 
	m_Info.setProcessID(ival);
}

void AppInfo::restoreProcInfo()
{
	m_pid = "-1";					//pid 值
	m_cpu_usage = "0%";				//cpu 占用率
	m_alivetime = "0 sec";			//已运行时间
	m_memory_usage = "0 KB";			//内存用量
	m_status = STATUS_ERROR;				//运行状态
}

string AppInfo::DumpProcInfo()
{
	string strRet = "";

	char stime[32];
	time_t current = time(NULL);	// 获得当前时间
	struct tm local_time;
#ifndef	linux
	int err_code = localtime_s(&local_time, &current);
#else
	localtime_r(&current,&local_time);
#endif
	strftime(stime, 32, "%Y-%m-%d %H:%M:%S", &local_time);
	m_updatetime = string(stime);
	strRet.append("CollectorTime|");
	strRet.append(stime);
	strRet.append("|appid|");
	strRet.append(m_appid);
	strRet.append("|Name|");
	strRet.append(m_name);
	strRet.append("|pid|");
	strRet.append(m_pid);
	strRet.append("|cpu usage|");
	strRet.append(m_cpu_usage);
	strRet.append("|alivetime|");
	strRet.append(m_alivetime);
	strRet.append("|memory usage|");
	strRet.append(m_memory_usage);
	strRet.append("|status|");
	strRet.append(m_status);
	m_updatetime = strRet;
	return strRet;
}

void AppInfo::update_Cpu_usage()
{
	setCpu_usage((int)m_Info.get_cpu_usage());
}


std::string AppInfo::getAllinfo() const
{
	return m_allinfo;
}


std::string AppInfo::getUpdatetime() const
{
	return m_updatetime;
}

void AppInfo::setVersion(string version)
{
	m_version = version;
}

std::string AppInfo::getVersion() const
{
	return m_version;
}

#ifdef linux
void AppInfo::update_Mem_usage()
{
	setMemory_usageKB((long)m_Info.get_mem_usage()*1024);
}

#endif


int AppInfo::getAppidInt() const
{
	return atol(m_appid.c_str());
}

int AppInfo::getStatusInt() const
{
	return atol(m_status.c_str());
}

void AppInfo::setStartime( string startime )
{
	m_startime = startime;
}

std::string AppInfo::getStartime() const
{
	return m_startime;
}

std::string AppInfo::GetFmtProcInfo()
{
	string ret = string("");
	ret.append("/M");
	ret.append(m_memory_usage.c_str());
	ret.append("/I");

	return ret;
}