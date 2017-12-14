#ifndef GetProcessInfo_h__
#define GetProcessInfo_h__

#ifndef linux

#pragma once
#include <string>
#include <list>

using namespace std;

class GetProcessInfo
{
public:
	GetProcessInfo(void);
	~GetProcessInfo(void);

private:
	string m_ProcessName; 
	LWORD m_ProcessID;
	//cpu数量
	int m_processor_count_;
	//上一次的时间
	__int64  m_last_time_;
	__int64  m_last_system_time_;

public:
	void cleanCountInfo();
	void setProcessName(string name);
	void setProcessNameNoQuery(string name);
	BOOL getProcessID();
	void setProcessID(LWORD wdID);
	double  get_cpu_usage();
	int	 getProcessNumber();
	__int64 FileTimeToInt64(const FILETIME& time);
};

#else

#pragma once
#include <string>
#include <time.h>
// this file deine some struct of sys/proc info
#include "Lnxprocdef.h"
using namespace std;


#define	MAXCNT	64

class GetProcessInfo
{
public:
	GetProcessInfo(void);
	~GetProcessInfo(void);

//--------------------------------div-----------static begin-----------------------------------------
public:
	/*
	** prototypes static
	*/
	//take a photo of system status
	static void photosyst();
	static void photosyst(struct sstat *si);
	static void	deviatsyst(struct sstat *cur, struct sstat *pre, struct sstat *dev);
	static void	totalsyst(char category, struct sstat *newst, struct sstat *tot);


	/*******************************help function********************************/

	static void getSupportInfo();
	static time_t getbootlinux(long hertz);
	static time_t getboot(void);
	static time_t getProcessUptimeByPid(int pid);
	static string getProcessStartimeByPid(int pid);
	/*******************************help function********************************/

	/*
	** open file "stat" and obtain required info
	*/
	static int procstat(struct tstat *curtask, time_t bootepoch, char isproc);
	/*
	** open file "status" and obtain required info
	*/
	static int procstatus(struct tstat *curtask);
	/*
	** store the full command line; the command-line may contain:
	**    - null-bytes as a separator between the arguments
	**    - newlines (e.g. arguments for awk or sed)
	**    - tabs (e.g. arguments for awk or sed)
	** these special bytes will be converted to spaces
	*/
	static void proccmd(struct tstat *curtask);
	/*
	** open file "io" (>= 2.6.20) and obtain required info
	*/
	static int procio(struct tstat *curtask);


public:
	static char gc_part_stats; /* per-partition statistics ? */

	//static struct sstat	*cg_phlpsstat;
	//static struct sstat	*cg_pdevsstat; /* deviation */
	//static struct sstat *cg_pPresstat; //previous
	static struct sstat *cg_pCursstat; //current

	//const by initclass
	static unsigned int cg_pagesize;
	static unsigned int cg_hertz;
	static time_t		cg_bootepoch;
	static int			cg_supportflags;	/* supported features             	*/
//--------------------------------div----------static end------------------------------------------

public:
	int photoproc();
	double get_cpu_usage();
	count_t get_mem_usage();

public:
	void setProcessName(string name){ m_ProcessName = name ;};
	void setProcessID(int pid){
		if(pid != m_ProcessID )
		{
			m_ProcessID = pid;
			cleanCountInfo();
		}
	};

private:
	void cleanCountInfo();

private:
	string	m_ProcessName;
	int		m_ProcessID;
	tstat 	m_p_stat;		//pre stat
	tstat 	m_c_stat;		//cur stat

};

#endif // ndef linux

#endif // GetProcessInfo_h__
