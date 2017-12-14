#ifndef AppInfo_h__
#define AppInfo_h__

#include "Tool.h"
#include "GetProcessInfo.h"
#include <string>
#include <list>
using namespace std;

#define DEFAULT_RECVPORT	    9105
#define DEFAULT_NODETYPE	    1001
#define DEFAULT_CHECKSPAN	    60
#define DEFAULT_LOG_LEVEL	    4
#define DEFAULT_LOG_MAXCOUNT	100000

#define STATUS_ERROR            "Error"

class AppInfo
{
public:
	AppInfo();
	//�������캯��
	AppInfo(const AppInfo& obj);

	virtual ~AppInfo(){};
	//����������
	AppInfo& operator=(const AppInfo &obj);

	string getAppid() const;
	int getAppidInt() const;

	string getName() const;

	string getPidfile() const;

	string getStartcmd() const;

	string getStopcmd() const;


	void setAppid(string strAppid);

	void setName(string strName);

	void setPidfile(string strPidfile);

	void setStartcmd(string strStartcmd);

	void setStopcmd(string strStopcmd);

	string getStatus() const;
	int getStatusInt() const;
	void setStatus(string val);
	void setStatus(int ival);

	string getMemory_usage() const;
	void setMemory_usage(string val);
	void setMemory_usageAT(int ival);
	void setMemory_usageKB(int ival);

	string getAlivetime() const;
	void setAlivetime(string val);

	// add by zdx 2012-09-11
	void setVersion(string version);
	string getVersion() const;

	//************************************
	// Method:    Alivetime
	// FullName:  AppInfo::Alivetime
	// Access:    public 
	// Returns:   void
	// Qualifier: ��
	// Parameter: long ival
	//************************************
#ifndef linux
	void setAlivetime(long upt);
#else
	void setAlivetime(time_t upt);
#endif


	string getCpu_usage() const;
	void setCpu_usage(string val);

	void setCpu_usage(int ival);

	void update_Cpu_usage();
#ifdef linux
	void update_Mem_usage();
#endif

	string getPid() const;
	int getPidInt() const;
	void setPid(string val);
	void setPid(int ival);

	string getAllinfo() const;
	string getUpdatetime() const;

	void setStartime(string startime);
	string getStartime() const;
	

	void restoreProcInfo();

	string DumpProcInfo();

	string GetFmtProcInfo();

	
protected:
	string m_appid;					//ģ��ID
	string m_name;					//�ļ�����
	string m_pidfile;				//pid�ļ�·��
	string m_startcmd;				//��������
	string m_stopcmd;				//��ֹ����--��ʱ����

	string m_pid;					//pid ֵ
	string m_cpu_usage;				//cpu ռ����
	string m_alivetime;				//������ʱ��
	string m_startime;				//������ʱ��
	string m_memory_usage;			//�ڴ�����
	string m_status;				//����״̬
	GetProcessInfo m_Info;			//cpuռ����
	string m_version;				//����汾			//add by zdx 2012-07-11

	//�����ⲿ���û�ȡ
	string m_updatetime;			
	string m_allinfo;
};
#endif // AppInfo_h__
