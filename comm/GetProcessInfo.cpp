

#include "GetProcessInfo.h"

#ifndef linux  // for windows

#include <TlHelp32.h>
#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>

#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Psapi.lib")

GetProcessInfo::GetProcessInfo(void)
{
	m_ProcessID = 0;
	m_ProcessName = "";
	//cpu数量
	m_processor_count_ = -1;
	//上一次的时间
	m_last_time_ = 0;
	m_last_system_time_ = 0;
}

GetProcessInfo::~GetProcessInfo(void)
{
}

// 获得进程名字
void GetProcessInfo::setProcessName(string name)
{
	m_ProcessName = name;
	getProcessID();
}

// 获得进程ID
BOOL GetProcessInfo::getProcessID()
{
	int iLeng = 0;
	m_ProcessID = NULL;
	PROCESSENTRY32 pe;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	pe.dwSize = sizeof(PROCESSENTRY32);

	if (hSnapshot==NULL)	
	{
		return FALSE;
	}

	const char* ProcessName = m_ProcessName.c_str();
	iLeng = strlen(ProcessName);
	Process32First(hSnapshot,&pe);
	
	//char word_1 ='';
	char word_2 = 0x00;

	do 
	{
		for (int i = 0; i < iLeng; i++)
		{
			if (pe.szExeFile[i] > 64 && pe.szExeFile[i] < 90)
			{
				word_2 = pe.szExeFile[i] + 32;
			}
			else
				word_2 = pe.szExeFile[i];

			if (m_ProcessName[i] != word_2)
			{
				break;
			}

			if (i == iLeng - 1 && m_ProcessName[i] == word_2)
			{
				m_ProcessID = pe.th32ProcessID;
				return TRUE;
			}
		}
	} while (Process32Next(hSnapshot,&pe));

	CloseHandle(hSnapshot);

	return FALSE;
}
 
int GetProcessInfo::getProcessNumber()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (int)info.dwNumberOfProcessors;
}

// 时间格式转换
__int64 GetProcessInfo::FileTimeToInt64(const FILETIME& time)
{
	ULARGE_INTEGER tt;
	tt.LowPart = time.dwLowDateTime;
	tt.HighPart = time.dwHighDateTime;
	return(tt.QuadPart);
}

double GetProcessInfo::get_cpu_usage()
{  
	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;
	__int64 system_time;
	__int64 time;
// 	__int64 system_time_delta;
// 	__int64 time_delta;

	double cpu = -1;

	if(m_processor_count_ == -1)
	{
		m_processor_count_ = getProcessNumber();
	}

	GetSystemTimeAsFileTime(&now);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION/*PROCESS_ALL_ACCESS*/, false, m_ProcessID);
	if (!hProcess)
	{
		return -1;
	}
	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		return -1;
	}
	system_time = (FileTimeToInt64(kernel_time) + FileTimeToInt64(user_time)) / m_processor_count_;  //CPU使用时间
	time = FileTimeToInt64(now);		//现在的时间

	m_last_system_time_ = system_time;
	m_last_time_ = time;
	CloseHandle( hProcess );

	Sleep(1000);

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION/*PROCESS_ALL_ACCESS*/, false, m_ProcessID);
	if (!hProcess)
	{
		return -1;
	}
	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		return -1;
	}
	GetSystemTimeAsFileTime(&now);
	system_time = (FileTimeToInt64(kernel_time) + FileTimeToInt64(user_time)) / m_processor_count_;  //CPU使用时间
	time = FileTimeToInt64(now);		//现在的时间

	CloseHandle( hProcess );

	cpu = ((double)(system_time - m_last_system_time_) / (double)(time - m_last_time_)) * 100;
	return cpu;
}

void GetProcessInfo::setProcessNameNoQuery( string name )
{
	m_ProcessName = name;
}

void GetProcessInfo::setProcessID( LWORD wdID )
{
	if(m_ProcessID != wdID)
	{
		cleanCountInfo();
		m_ProcessID = wdID;
	}
	else
		return;
}

void GetProcessInfo::cleanCountInfo()
{
	//cpu数量
	m_processor_count_ = -1;
	//上一次的时间
	m_last_time_ = 0;
	m_last_system_time_ = 0;
}

#else
#include<string.h>
#include<stdlib.h>
#include<iostream>
#include<unistd.h>
#include<dirent.h>
#include<sys/wait.h>
#include "GetProcessInfo.h"
#include "Lnxprocdef.h"
using namespace std;


char GetProcessInfo::gc_part_stats; /* per-partition statistics ? */
//struct sstat* GetProcessInfo::cg_phlpsstat = new sstat;
//struct sstat* GetProcessInfo::cg_pdevsstat = new sstat; /* deviation */
//struct sstat* GetProcessInfo::cg_pPresstat = new sstat; //previous
struct sstat* GetProcessInfo::cg_pCursstat = new sstat; //current

time_t GetProcessInfo::cg_bootepoch=0;
unsigned int GetProcessInfo::cg_pagesize = sysconf(_SC_PAGESIZE);
unsigned int GetProcessInfo::cg_hertz = sysconf(_SC_CLK_TCK);
int GetProcessInfo::cg_supportflags = 0x00;


GetProcessInfo::GetProcessInfo(void)
{

}
GetProcessInfo::~GetProcessInfo(void)
{

}
//--------------------------------div-----------static begin-----------------------------------------
/*
** prototypes static
*/
//take a photo of system status
void GetProcessInfo::photosyst()
{
	return photosyst(cg_pCursstat);
}
//take a photo of system status
void GetProcessInfo::photosyst(struct sstat *si)
{
	register int	i, nr;
	count_t		cnts[MAXCNT];
	float		lavg1, lavg5, lavg15;
	FILE 		*fp;
	char		linebuf[1024], nam[64], origdir[1024];
	gc_part_stats = 1; /* per-partition statistics ? */
	//unsigned int	major, minor;

	memset(si, 0, sizeof(struct sstat));

	getcwd(origdir, sizeof origdir);
	chdir("/proc");

	/*
	** gather various general statistics from the file /proc/stat and
	** store them in binary form
	*/
	if ( (fp = fopen("stat", "r")) != NULL)
	{
		while ( fgets(linebuf, sizeof(linebuf), fp) != NULL)
		{
			nr = sscanf(linebuf,
			            "%s   %lld %lld %lld %lld %lld %lld %lld "
			            "%lld %lld %lld %lld %lld %lld %lld %lld ",
			  	nam,
			  	&cnts[0],  &cnts[1],  &cnts[2],  &cnts[3],
			  	&cnts[4],  &cnts[5],  &cnts[6],  &cnts[7],
			  	&cnts[8],  &cnts[9],  &cnts[10], &cnts[11],
			  	&cnts[12], &cnts[13], &cnts[14]);

			if (nr < 2)		/* headerline ? --> skip */
				continue;

			if ( strcmp("cpu", nam) == EQ)
			{
				si->cpu.all.utime	= cnts[0];
				si->cpu.all.ntime	= cnts[1];
				si->cpu.all.stime	= cnts[2];
				si->cpu.all.itime	= cnts[3];

				if (nr > 5)	/* 2.6 kernel? */
				{
					si->cpu.all.wtime	= cnts[4];
					si->cpu.all.Itime	= cnts[5];
					si->cpu.all.Stime	= cnts[6];

					if (nr > 8)	/* steal support */
						si->cpu.all.steal = cnts[7];

					if (nr > 9)	/* guest support */
						si->cpu.all.guest = cnts[8];
				}
				continue;
			}

			if ( strncmp("cpu", nam, 3) == EQ)
			{
				i = atoi(&nam[3]);

				if (i >= MAXCPU)
				{
					fprintf(stderr,
						"cpu %s exceeds maximum of %d\n",
						nam, MAXCPU);
					continue;
				}

				si->cpu.cpu[i].cpunr	= i;
				si->cpu.cpu[i].utime	= cnts[0];
				si->cpu.cpu[i].ntime	= cnts[1];
				si->cpu.cpu[i].stime	= cnts[2];
				si->cpu.cpu[i].itime	= cnts[3];

				if (nr > 5)	/* 2.6 kernel? */
				{
					si->cpu.cpu[i].wtime	= cnts[4];
					si->cpu.cpu[i].Itime	= cnts[5];
					si->cpu.cpu[i].Stime	= cnts[6];

					if (nr > 8)	/* steal support */
						si->cpu.cpu[i].steal = cnts[7];

					if (nr > 9)	/* guest support */
						si->cpu.cpu[i].guest = cnts[8];
				}

				si->cpu.nrcpu++;
				continue;
			}

			if ( strcmp("ctxt", nam) == EQ)
			{
				si->cpu.csw	= cnts[0];
				continue;
			}

			if ( strcmp("intr", nam) == EQ)
			{
				si->cpu.devint	= cnts[0];
				continue;
			}

			if ( strcmp("processes", nam) == EQ)
			{
				si->cpu.nprocs	= cnts[0];
				continue;
			}

			if ( strcmp("swap", nam) == EQ)   /* < 2.6 */
			{
				si->mem.swins	= cnts[0];
				si->mem.swouts	= cnts[1];
				continue;
			}
		}

		fclose(fp);

		if (si->cpu.nrcpu == 0)
			si->cpu.nrcpu = 1;
	}

	/*
	** gather loadaverage values from the file /proc/loadavg and
	** store them in binary form
	*/
	if ( (fp = fopen("loadavg", "r")) != NULL)
	{
		if ( fgets(linebuf, sizeof(linebuf), fp) != NULL)
		{
			if ( sscanf(linebuf, "%f %f %f",
				&lavg1, &lavg5, &lavg15) == 3)
			{
				si->cpu.lavg1	= lavg1;
				si->cpu.lavg5	= lavg5;
				si->cpu.lavg15	= lavg15;
			}
		}

		fclose(fp);
	}

	/*
	** gather frequency scaling info.
        ** sources (in order of preference):
        **     /sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state
        ** or
        **     /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
        **     /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
        **
	** store them in binary form
	*/
        static char fn[80];
        int didone=0;

        for (i = 0; i < si->cpu.nrcpu; ++i)
        {
                long long f=0;

                sprintf(fn,
                   "/sys/devices/system/cpu/cpu%d/cpufreq/stats/time_in_state",
                   i);

                if ((fp=fopen(fn, "r")) != 0)
                {
                        long long hits=0;
                        long long maxfreq=0;
                        long long cnt=0;
                        long long sum=0;

                        while (fscanf(fp, "%lld %lld", &f, &cnt) == 2)
                        {
                                f	/= 1000;
                                sum 	+= (f*cnt);
                                hits	+= cnt;

                                if (f > maxfreq)
                        		maxfreq=f;
                        }

	                si->cpu.cpu[i].freqcnt.maxfreq	= maxfreq;
	                si->cpu.cpu[i].freqcnt.cnt	= sum;
	                si->cpu.cpu[i].freqcnt.ticks	= hits;

                        fclose(fp);
                        didone=1;
                }
		else
		{    // governor statistics not available
                     sprintf(fn,
                      "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq",
		      i);

                        if ((fp=fopen(fn, "r")) != 0)
                        {
                                if (fscanf(fp, "%lld", &f) == 1)
                                {
  					// convert KHz to MHz
	                                si->cpu.cpu[i].freqcnt.maxfreq =f/1000;
                                }

                                didone=1;
                                fclose(fp);
                        }
                        else
                        {
	                        si->cpu.cpu[i].freqcnt.maxfreq=0;
                        }

                       sprintf(fn,
                       "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq",
		       i);

                        if ((fp=fopen(fn, "r")) != 0)
                        {
                                if (fscanf(fp, "%lld", &f) == 1)
                                {
   					// convert KHz to MHz
                                        si->cpu.cpu[i].freqcnt.cnt   = f/1000;
                                        si->cpu.cpu[i].freqcnt.ticks = 0;
                                }

                                fclose(fp);
                                didone=1;
                        }
                        else
                        {
                                si->cpu.cpu[i].freqcnt.maxfreq	= 0;
                                si->cpu.cpu[i].freqcnt.cnt	= 0;
                                si->cpu.cpu[i].freqcnt.ticks 	= 0;
                                break;    // use cpuinfo
                        }
                }
        } // for all CPUs

        if (!didone)     // did not get processor freq statistics.
                         // use /proc/cpuinfo
        {
	        if ( (fp = fopen("cpuinfo", "r")) != NULL)
                {
                        // get information from the lines
                        // processor\t: 0
                        // cpu MHz\t\t: 800.000

                        int cpuno=-1;

		        while ( fgets(linebuf, sizeof(linebuf), fp) != NULL)
                        {
                                if (memcmp(linebuf, "processor", 9)== EQ)
					sscanf(linebuf, "%*s %*s %d", &cpuno);

                                if (memcmp(linebuf, "cpu MHz", 7) == EQ)
				{
                                        if (cpuno >= 0 && cpuno < si->cpu.nrcpu)
					{
						sscanf(linebuf,
							"%*s %*s %*s %lld",
                                	     		&(si->cpu.cpu[cpuno].freqcnt.cnt));
					}
                                }
                        }

			fclose(fp);
                }

        }

	/*
	** gather virtual memory statistics from the file /proc/vmstat and
	** store them in binary form (>= kernel 2.6)
	*/
	if ( (fp = fopen("vmstat", "r")) != NULL)
	{
		while ( fgets(linebuf, sizeof(linebuf), fp) != NULL)
		{
			nr = sscanf(linebuf, "%s %lld", nam, &cnts[0]);

			if (nr < 2)		/* headerline ? --> skip */
				continue;

			if ( strcmp("pswpin", nam) == EQ)
			{
				si->mem.swins   = cnts[0];
				continue;
			}

			if ( strcmp("pswpout", nam) == EQ)
			{
				si->mem.swouts  = cnts[0];
				continue;
			}

			if ( strncmp("pgscan_", nam, 7) == EQ)
			{
				si->mem.pgscans += cnts[0];
				continue;
			}

			if ( strncmp("pgsteal_", nam, 8) == EQ)
			{
				si->mem.pgsteal += cnts[0];
				continue;
			}

			if ( strcmp("allocstall", nam) == EQ)
			{
				si->mem.allocstall = cnts[0];
				continue;
			}
		}

		fclose(fp);
	}

	/*
	** gather memory-related statistics from the file /proc/meminfo and
	** store them in binary form
	**
	** in the file /proc/meminfo a 2.4 kernel starts with two lines
	** headed by the strings "Mem:" and "Swap:" containing all required
	** fields, except proper value for page cache
        ** if these lines are present we try to skip parsing the rest
	** of the lines; if these lines are not present we should get the
	** required field from other lines
	*/
	si->mem.physmem	 	= (count_t)-1;
	si->mem.freemem		= (count_t)-1;
	si->mem.buffermem	= (count_t)-1;
	si->mem.cachemem  	= (count_t)-1;
	si->mem.slabmem		= (count_t) 0;
	si->mem.totswap  	= (count_t)-1;
	si->mem.freeswap 	= (count_t)-1;
	si->mem.committed 	= (count_t) 0;

	if ( (fp = fopen("meminfo", "r")) != NULL)
	{
		int	nrfields = 10;	/* number of fields to be filled */

		while ( fgets(linebuf, sizeof(linebuf), fp) != NULL &&
								nrfields > 0)
		{
			nr = sscanf(linebuf,
				"%s %lld %lld %lld %lld %lld %lld %lld "
			        "%lld %lld %lld\n",
				nam,
			  	&cnts[0],  &cnts[1],  &cnts[2],  &cnts[3],
			  	&cnts[4],  &cnts[5],  &cnts[6],  &cnts[7],
			  	&cnts[8],  &cnts[9]);

			if (nr < 2)		/* headerline ? --> skip */
				continue;

			if ( strcmp("Mem:", nam) == EQ)
			{
				si->mem.physmem	 	= cnts[0] / cg_pagesize;
				si->mem.freemem		= cnts[2] / cg_pagesize;
				si->mem.buffermem	= cnts[4] / cg_pagesize;
				nrfields -= 3;
			}
			else	if ( strcmp("Swap:", nam) == EQ)
				{
					si->mem.totswap  = cnts[0] / cg_pagesize;
					si->mem.freeswap = cnts[2] / cg_pagesize;
					nrfields -= 2;
				}
			else	if (strcmp("Cached:", nam) == EQ)
				{
					if (si->mem.cachemem  == (count_t)-1)
					{
						si->mem.cachemem  =
							cnts[0]*1024/cg_pagesize;
						nrfields--;
					}
				}
			else	if (strcmp("Dirty:", nam) == EQ)
				{
					si->mem.cachedrt  =
							cnts[0]*1024/cg_pagesize;
					nrfields--;
				}
			else	if (strcmp("MemTotal:", nam) == EQ)
				{
					if (si->mem.physmem  == (count_t)-1)
					{
						si->mem.physmem  =
							cnts[0]*1024/cg_pagesize;
						nrfields--;
					}
				}
			else	if (strcmp("MemFree:", nam) == EQ)
				{
					if (si->mem.freemem  == (count_t)-1)
					{
						si->mem.freemem  =
							cnts[0]*1024/cg_pagesize;
						nrfields--;
					}
				}
			else	if (strcmp("Buffers:", nam) == EQ)
				{
					if (si->mem.buffermem  == (count_t)-1)
					{
						si->mem.buffermem  =
							cnts[0]*1024/cg_pagesize;
						nrfields--;
					}
				}
			else	if (strcmp("SwapTotal:", nam) == EQ)
				{
					if (si->mem.totswap  == (count_t)-1)
					{
						si->mem.totswap  =
							cnts[0]*1024/cg_pagesize;
						nrfields--;
					}
				}
			else	if (strcmp("SwapFree:", nam) == EQ)
				{
					if (si->mem.freeswap  == (count_t)-1)
					{
						si->mem.freeswap  =
							cnts[0]*1024/cg_pagesize;
						nrfields--;
					}
				}
			else	if (strcmp("Slab:", nam) == EQ)
				{
					si->mem.slabmem = cnts[0]*1024/cg_pagesize;
					nrfields--;
				}
			else	if (strcmp("Committed_AS:", nam) == EQ)
				{
					si->mem.committed = cnts[0]*1024/
								cg_pagesize;
					nrfields--;
				}
			else	if (strcmp("CommitLimit:", nam) == EQ)
				{
					si->mem.commitlim = cnts[0]*1024/
								cg_pagesize;
					nrfields--;
				}
		}

		fclose(fp);
	}
}

/*
** calculate the system-activity during the last sample
*/
void GetProcessInfo::deviatsyst(struct sstat *cur, struct sstat *pre, struct sstat *dev)
{
	register int	i;
	//count_t		*cdev, *ccur, *cpre;

	dev->cpu.nrcpu     = cur->cpu.nrcpu;
	dev->cpu.devint    = subcount(cur->cpu.devint, pre->cpu.devint);
	dev->cpu.csw       = subcount(cur->cpu.csw,    pre->cpu.csw);
	dev->cpu.nprocs    = subcount(cur->cpu.nprocs, pre->cpu.nprocs);

	dev->cpu.all.stime = subcount(cur->cpu.all.stime, pre->cpu.all.stime);
	dev->cpu.all.utime = subcount(cur->cpu.all.utime, pre->cpu.all.utime);
	dev->cpu.all.ntime = subcount(cur->cpu.all.ntime, pre->cpu.all.ntime);
	dev->cpu.all.itime = subcount(cur->cpu.all.itime, pre->cpu.all.itime);
	dev->cpu.all.wtime = subcount(cur->cpu.all.wtime, pre->cpu.all.wtime);
	dev->cpu.all.Itime = subcount(cur->cpu.all.Itime, pre->cpu.all.Itime);
	dev->cpu.all.Stime = subcount(cur->cpu.all.Stime, pre->cpu.all.Stime);

	dev->cpu.all.steal = subcount(cur->cpu.all.steal, pre->cpu.all.steal);
	dev->cpu.all.guest = subcount(cur->cpu.all.guest, pre->cpu.all.guest);

	for (i=0; i < dev->cpu.nrcpu; i++)
	{
		count_t 	ticks;

		dev->cpu.cpu[i].cpunr = cur->cpu.cpu[i].cpunr;
		dev->cpu.cpu[i].stime = subcount(cur->cpu.cpu[i].stime,
					         pre->cpu.cpu[i].stime);
		dev->cpu.cpu[i].utime = subcount(cur->cpu.cpu[i].utime,
				 	         pre->cpu.cpu[i].utime);
		dev->cpu.cpu[i].ntime = subcount(cur->cpu.cpu[i].ntime,
					         pre->cpu.cpu[i].ntime);
		dev->cpu.cpu[i].itime = subcount(cur->cpu.cpu[i].itime,
					         pre->cpu.cpu[i].itime);
		dev->cpu.cpu[i].wtime = subcount(cur->cpu.cpu[i].wtime,
					         pre->cpu.cpu[i].wtime);
		dev->cpu.cpu[i].Itime = subcount(cur->cpu.cpu[i].Itime,
					         pre->cpu.cpu[i].Itime);
		dev->cpu.cpu[i].Stime = subcount(cur->cpu.cpu[i].Stime,
					         pre->cpu.cpu[i].Stime);

		dev->cpu.cpu[i].steal = subcount(cur->cpu.cpu[i].steal,
					         pre->cpu.cpu[i].steal);
		dev->cpu.cpu[i].guest = subcount(cur->cpu.cpu[i].guest,
					         pre->cpu.cpu[i].guest);

		ticks 		      = cur->cpu.cpu[i].freqcnt.ticks;

		dev->cpu.cpu[i].freqcnt.maxfreq =
					cur->cpu.cpu[i].freqcnt.maxfreq;
		dev->cpu.cpu[i].freqcnt.cnt = ticks ?
					subcount(cur->cpu.cpu[i].freqcnt.cnt,
					         pre->cpu.cpu[i].freqcnt.cnt)
					       : cur->cpu.cpu[i].freqcnt.cnt;

		dev->cpu.cpu[i].freqcnt.ticks = ticks ?
					subcount(cur->cpu.cpu[i].freqcnt.ticks,
					         pre->cpu.cpu[i].freqcnt.ticks)
					       : cur->cpu.cpu[i].freqcnt.ticks;
	}

	dev->cpu.lavg1		= cur->cpu.lavg1;
	dev->cpu.lavg5		= cur->cpu.lavg5;
	dev->cpu.lavg15		= cur->cpu.lavg15;

	dev->mem.physmem	= cur->mem.physmem;
	dev->mem.freemem	= cur->mem.freemem;
	dev->mem.buffermem	= cur->mem.buffermem;
	dev->mem.slabmem	= cur->mem.slabmem;
	dev->mem.committed	= cur->mem.committed;
	dev->mem.commitlim	= cur->mem.commitlim;
	dev->mem.cachemem	= cur->mem.cachemem;
	dev->mem.cachedrt	= cur->mem.cachedrt;
	dev->mem.totswap	= cur->mem.totswap;
	dev->mem.freeswap	= cur->mem.freeswap;

	dev->mem.swouts		= subcount(cur->mem.swouts,  pre->mem.swouts);
	dev->mem.swins		= subcount(cur->mem.swins,   pre->mem.swins);
	dev->mem.pgscans	= subcount(cur->mem.pgscans, pre->mem.pgscans);
	dev->mem.pgsteal	= subcount(cur->mem.pgsteal, pre->mem.pgsteal);
	dev->mem.allocstall	= subcount(cur->mem.allocstall,
				                         pre->mem.allocstall);
}

/*
** add the values of a new sample to a structure holding the totals
** for the indicated category (c=cpu, m=memory, d=disk, n=network).
*/
void GetProcessInfo::totalsyst (char category, struct sstat *newst, struct sstat *tot)
{
	register int	i;
	//count_t		*ctot, *cnew;

	switch (category)
	{
	   case 'c':	/* accumulate cpu-related counters */
		tot->cpu.nrcpu      = newst->cpu.nrcpu;
		tot->cpu.devint    += newst->cpu.devint;
		tot->cpu.csw       += newst->cpu.csw;
		tot->cpu.nprocs    += newst->cpu.nprocs;

		tot->cpu.all.stime += newst->cpu.all.stime;
		tot->cpu.all.utime += newst->cpu.all.utime;
		tot->cpu.all.ntime += newst->cpu.all.ntime;
		tot->cpu.all.itime += newst->cpu.all.itime;
		tot->cpu.all.wtime += newst->cpu.all.wtime;
		tot->cpu.all.Itime += newst->cpu.all.Itime;
		tot->cpu.all.Stime += newst->cpu.all.Stime;
		tot->cpu.all.steal += newst->cpu.all.steal;
		tot->cpu.all.guest += newst->cpu.all.guest;

		if (newst->cpu.nrcpu == 1)
		{
			tot->cpu.cpu[0] = tot->cpu.all;
		}
		else
		{
			for (i=0; i < newst->cpu.nrcpu; i++)
			{
				tot->cpu.cpu[i].cpunr  = newst->cpu.cpu[i].cpunr;
				tot->cpu.cpu[i].stime += newst->cpu.cpu[i].stime;
				tot->cpu.cpu[i].utime += newst->cpu.cpu[i].utime;
				tot->cpu.cpu[i].ntime += newst->cpu.cpu[i].ntime;
				tot->cpu.cpu[i].itime += newst->cpu.cpu[i].itime;
				tot->cpu.cpu[i].wtime += newst->cpu.cpu[i].wtime;
				tot->cpu.cpu[i].Itime += newst->cpu.cpu[i].Itime;
				tot->cpu.cpu[i].Stime += newst->cpu.cpu[i].Stime;
				tot->cpu.cpu[i].steal += newst->cpu.cpu[i].steal;
				tot->cpu.cpu[i].guest += newst->cpu.cpu[i].guest;
			}
		}

		tot->cpu.lavg1	 = newst->cpu.lavg1;
		tot->cpu.lavg5	 = newst->cpu.lavg5;
		tot->cpu.lavg15	 = newst->cpu.lavg15;
		break;

	   case 'm':	/* accumulate memory-related counters */
		tot->mem.physmem	 = newst->mem.physmem;
		tot->mem.freemem	 = newst->mem.freemem;
		tot->mem.buffermem	 = newst->mem.buffermem;
		tot->mem.slabmem	 = newst->mem.slabmem;
		tot->mem.committed	 = newst->mem.committed;
		tot->mem.commitlim	 = newst->mem.commitlim;
		tot->mem.cachemem	 = newst->mem.cachemem;
		tot->mem.cachedrt	 = newst->mem.cachedrt;
		tot->mem.totswap	 = newst->mem.totswap;
		tot->mem.freeswap	 = newst->mem.freeswap;

		tot->mem.swouts		+= newst->mem.swouts;
		tot->mem.swins		+= newst->mem.swins;
		tot->mem.pgscans	+= newst->mem.pgscans;
		tot->mem.allocstall	+= newst->mem.allocstall;
		break;
	}

}


/*********************help function********************************/

void GetProcessInfo::getSupportInfo()
{
	FILE    	*fp;
	/*
	** check if this kernel is patched for additional
	** per-task counters
	*/
	if ( (fp = fopen("/proc/1/stat", "r")) )
	{
		char	line[4096];

		/*
		** when the patch is installed, the output
		** of /proc/pid/stat contains two lines
		*/
		(void) fgets(line, sizeof line, fp);

		if ( fgets(line, sizeof line, fp) != NULL)
			cg_supportflags |= PATCHSTAT;

		fclose(fp);
	}

	/*
	** check if this kernel offers io-statistics per task
	*/
	if ( !(cg_supportflags & PATCHSTAT) )
	{
		if ( (fp = fopen("/proc/1/io", "r")) )
		{
			cg_supportflags |= IOSTAT;

			fclose(fp);
		}
	}
	/*
		** find epoch time of boot moment
	*/
	cg_bootepoch = getboot();
}

/*
** LINUX SPECIFIC:
** Determine boot-time of this system (as number of seconds since 1-1-1970).
*/

time_t  GetProcessInfo::getbootlinux(long hertz)
{
	int     	cpid;
	char  	  	tmpbuf[1280];
	FILE    	*fp;
	unsigned long 	startticks;
	time_t		boottime = 0;

	/*
	** dirty hack to get the boottime, since the
	** Linux 2.6 kernel (2.6.5) does not return a proper
	** boottime-value with the times() system call :-(
	*/
	if ( (cpid = fork()) == 0 )
	{
		/*
		** child just waiting to be killed by parent
		*/
		pause();
	}
	else
	{
		/*
		** parent determines start-time (in clock-ticks since boot)
		** of the child and calculates the boottime in seconds
		** since 1-1-1970
		*/
		snprintf(tmpbuf, sizeof tmpbuf, "/proc/%d/stat", cpid);

		if ( (fp = fopen(tmpbuf, "r")) != NULL)
		{
			if ( fscanf(fp, "%*d (%*[^)]) %*c %*d %*d %*d %*d "
			                "%*d %*d %*d %*d %*d %*d %*d %*d "
			                "%*d %*d %*d %*d %*d %*d %lu",
			                &startticks) == 1)
			{
				boottime = time(0) - startticks / hertz;
			}

			fclose(fp);
		}

		/*
		** kill the child and get rid of the zombie
		*/
		kill(cpid, SIGKILL);
		(void) wait((int *)0);
	}

	return boottime;
}


/*
** Function getboot() returns the boot-time of this system
** (as number of seconds since 1-1-1970).
*/
time_t GetProcessInfo::getboot(void)
{
	static time_t	boottime;

	if (!boottime)		/* do this only once */
	{
#ifdef	linux
		boottime = getbootlinux(cg_hertz);
#else
		struct tms	tms;
		time_t 		boottime_again;

		/*
		** beware that between time() and times() a one-second
		** upgrade could have taken place in the kernel, so an
		** extra check is issued; if we were around a
		** second-upgrade, just do it again (we just passed
		** the danger-zone)
		*/
		boottime 	= time(0) - (times(&tms) / cg_hertz);
		boottime_again	= time(0) - (times(&tms) / cg_hertz);

		if (boottime != boottime_again)
			boottime = time(0) - (times(&tms) / cg_hertz);
#endif
	}

	return boottime;
}

/*
** open file "stat" and obtain required info
*/
int GetProcessInfo::procstat(struct tstat *curtask, time_t bootepoch, char isproc)
{
	FILE	*fp;
	int	nr;
	char	line[4096], *cmdhead, *cmdtail;

	if ( (fp = fopen("stat", "r")) == NULL)
		return 0;

	if (fgets(line, sizeof line, fp) == NULL)
	{
		fclose(fp);
		return 0;
	}

	/*
    	** fetch command name
	*/
	cmdhead = strchr (line, '(');
	cmdtail = strrchr(line, ')');
	if ( (nr = cmdtail-cmdhead-1) > PNAMLEN)
		nr = PNAMLEN;

	memcpy(curtask->gen.name, cmdhead+1, nr);
	*(curtask->gen.name+nr) = 0;

	/*
  	** fetch other values
  	*/
	curtask->gen.isproc = isproc;
	curtask->cpu.rtprio  = 0;
	curtask->cpu.policy  = 0;
	curtask->gen.excode  = 0;

	sscanf(line, "%d", &(curtask->gen.pid));  /* fetch pid */

	nr = sscanf(cmdtail+2, SCANSTAT,
		&(curtask->gen.state), 	&(curtask->gen.ppid),
		&(curtask->mem.minflt),	&(curtask->mem.majflt),
		&(curtask->cpu.utime),	&(curtask->cpu.stime),
		&(curtask->cpu.prio),	&(curtask->cpu.nice),
		&(curtask->gen.btime),
		&(curtask->mem.vmem),	&(curtask->mem.rmem),
		&(curtask->cpu.curcpu),	&(curtask->cpu.rtprio),
		&(curtask->cpu.policy));

	if (nr < 12)		/* parsing failed? */
	{
		fclose(fp);
		return 0;
	}

	/*
 	** normalization
	*/
	curtask->gen.btime   = curtask->gen.btime/GetProcessInfo::cg_hertz+bootepoch;
	curtask->cpu.prio   += 100; 	/* was subtracted by kernel */
	curtask->mem.vmem   /= 1024;
	curtask->mem.rmem   *= GetProcessInfo::cg_pagesize/1024;

	/*
 	** second line present for patched kernel?
	*/
	if ( fgets(line, sizeof line, fp) != NULL)
	{
		sscanf(line, ATOPSTAT,
			&(curtask->dsk.rio),	&(curtask->dsk.rsz),
			&(curtask->dsk.wio),	&(curtask->dsk.wsz),
			&(curtask->net.tcpsnd),	&(curtask->net.tcpssz),
			&(curtask->net.tcprcv),	&(curtask->net.tcprsz),
			&(curtask->net.udpsnd),	&(curtask->net.udpssz),
			&(curtask->net.udprcv),	&(curtask->net.udprsz),
			&(curtask->net.rawsnd),	&(curtask->net.rawrcv));
	}

	fclose(fp);

	switch (curtask->gen.state)
	{
  	   case 'R':
		curtask->gen.nthrrun  = 1;
		break;
  	   case 'S':
		curtask->gen.nthrslpi = 1;
		break;
  	   case 'D':
		curtask->gen.nthrslpu = 1;
		break;
	}

	return 1;
}
/*
** open file "status" and obtain required info
*/
int GetProcessInfo::procstatus(struct tstat *curtask)
{
	FILE	*fp;
	char	line[4096];

	if ( (fp = fopen("status", "r")) == NULL)
		return 0;

	curtask->gen.nthr     = 1;	/* for compat with 2.4 */
	curtask->cpu.sleepavg = 0;	/* for compat with 2.4 */
	curtask->mem.vgrow    = 0;	/* calculated later */
	curtask->mem.rgrow    = 0;	/* calculated later */

	while (fgets(line, sizeof line, fp))
	{
		if (memcmp(line, "Tgid:", 5) ==0)
		{
			sscanf(line, "Tgid: %d", &(curtask->gen.tgid));
			continue;
		}

		if (memcmp(line, "Pid:", 4) ==0)
		{
			sscanf(line, "Pid: %d", &(curtask->gen.pid));
			continue;
		}

		if (memcmp(line, "SleepAVG:", 9)==0)
		{
			sscanf(line, "SleepAVG: %d%%",
				&(curtask->cpu.sleepavg));
			continue;
		}

		if (memcmp(line, "Uid:", 4)==0)
		{
			sscanf(line, "Uid: %d %d %d %d",
				&(curtask->gen.ruid), &(curtask->gen.euid),
				&(curtask->gen.suid), &(curtask->gen.fsuid));
			continue;
		}

		if (memcmp(line, "Gid:", 4)==0)
		{
			sscanf(line, "Gid: %d %d %d %d",
				&(curtask->gen.rgid), &(curtask->gen.egid),
				&(curtask->gen.sgid), &(curtask->gen.fsgid));
			continue;
		}

		if (memcmp(line, "Threads:", 8)==0)
		{
			sscanf(line, "Threads: %d", &(curtask->gen.nthr));
			continue;
		}

		if (memcmp(line, "VmData:", 7)==0)
		{
			sscanf(line, "VmData: %lld", &(curtask->mem.vdata));
			continue;
		}

		if (memcmp(line, "VmStk:", 6)==0)
		{
			sscanf(line, "VmStk: %lld", &(curtask->mem.vstack));
			continue;
		}

		if (memcmp(line, "VmExe:", 6)==0)
		{
			sscanf(line, "VmExe: %lld", &(curtask->mem.vexec));
			continue;
		}

		if (memcmp(line, "VmLib:", 6)==0)
		{
			sscanf(line, "VmLib: %lld", &(curtask->mem.vlibs));
			continue;
		}

		if (memcmp(line, "VmSwap:", 7)==0)
		{
			sscanf(line, "VmSwap: %lld", &(curtask->mem.vswap));
			continue;
		}
		if (memcmp(line, "SigQ:", 5)==0)
			break;
	}

	fclose(fp);
	return 1;
}
/*
** store the full command line; the command-line may contain:
**    - null-bytes as a separator between the arguments
**    - newlines (e.g. arguments for awk or sed)
**    - tabs (e.g. arguments for awk or sed)
** these special bytes will be converted to spaces
*/
void GetProcessInfo::proccmd(struct tstat *curtask)
{
	FILE		*fp;
	register int 	i, nr;

	memset(curtask->gen.cmdline, 0, CMDLEN+1);

	if ( (fp = fopen("cmdline", "r")) != NULL)
	{
		register char *p = curtask->gen.cmdline;

		nr = fread(p, 1, CMDLEN, fp);
		fclose(fp);

		if (nr >= 0)	/* anything read ? */
		{
			for (i=0; i < nr-1; i++, p++)
			{
				switch (*p)
				{
				   case '\0':
				   case '\n':
				   case '\t':
					*p = ' ';
					break;
				}
			}
		}
	}
}

/*
** open file "io" (>= 2.6.20) and obtain required info
*/
int GetProcessInfo::procio(struct tstat *curtask)
{
	FILE	*fp;
	char	line[4096];
	count_t	dskrsz = 0, dskwsz = 0, dskcwsz = 0;

	if (cg_supportflags & IOSTAT)
	{
		if ( (fp = fopen("io", "r")) )
		{
			while (fgets(line, sizeof line, fp))
			{
				if (memcmp(line, IO_READ,
						sizeof IO_READ -1) == 0)
				{
					sscanf(line, "%*s %llu", &dskrsz);
					dskrsz /= 512;		// in sectors
					continue;
				}

				if (memcmp(line, IO_WRITE,
						sizeof IO_WRITE -1) == 0)
				{
					sscanf(line, "%*s %llu", &dskwsz);
					dskwsz /= 512;		// in sectors
					continue;
				}

				if (memcmp(line, IO_CWRITE,
						sizeof IO_CWRITE -1) == 0)
				{
					sscanf(line, "%*s %llu", &dskcwsz);
					dskcwsz /= 512;		// in sectors
					continue;
				}
			}

			fclose(fp);

			curtask->dsk.rsz	= dskrsz;
			curtask->dsk.rio	= dskrsz;  // to enable sort
			curtask->dsk.wsz	= dskwsz;
			curtask->dsk.wio	= dskwsz;  // to enable sort
			curtask->dsk.cwsz	= dskcwsz;
		}
	}

	return 1;
}

time_t GetProcessInfo::getProcessUptimeByPid(int pid)
{
	char 		szPath[512];
	sprintf(szPath,"/proc/%d/cmdline",pid);
	return getProcessUptime(szPath);
}



/*********************help function********************************/

//--------------------------------div----------static end------------------------------------------

void GetProcessInfo::cleanCountInfo()
{
	memset(&m_c_stat,0x00,sizeof(tstat));
	//memset(&m_p_stat,0x00,sizeof(tstat));
	if(photoproc()!=0)
	{
		fprintf(stderr,"photoproc fail!!! pid=%d\n",m_ProcessID);
	}
}

int GetProcessInfo::photoproc()
{

	char		origdir[1024];
	char 		szPath[512];
	int		tval=0;

	sprintf(szPath,"/proc/%d",m_ProcessID);
	/*
	** read all subdirectory-names below the /proc directory
	*/
	getcwd(origdir, sizeof origdir);
	chdir("/proc");

	/*
	** change to the process' subdirectory
	*/
	if ( chdir(szPath) != 0 )
		return -1;


	if ( !GetProcessInfo::procstat(&m_c_stat, GetProcessInfo::cg_bootepoch, 1)) /* from /proc/pid/stat */
	{
		chdir("..");
		return -2;
	}


	if ( !GetProcessInfo::procstatus(&m_c_stat) )	    /* from /proc/pid/status  */
	{
		chdir("..");
		chdir(origdir);
		return -3;
	}


	//maybe not support
	if ( !GetProcessInfo::procio(&m_c_stat) )		    /* from /proc/pid/io      */
	{
		chdir("..");
	}



	GetProcessInfo::proccmd(&m_c_stat);		    /* from /proc/pid/cmdline */


	chdir("..");	/* leave process-level directry */

	chdir(origdir);

	return tval;
}

double GetProcessInfo::get_cpu_usage()
{
	sstat* pSys = GetProcessInfo::cg_pCursstat;
	sstat SysCur;
	//cpu 计算
	double perc = 0.00;
	GetProcessInfo::photosyst(pSys);
	if(photoproc()!=0)
	{
		fprintf(stderr,"photoproc fail!!! pid=%d\n",m_ProcessID);
		return perc;
	}
	sleep(1);
	GetProcessInfo::photosyst(&SysCur);
	if(photoproc()!=0)
	{
		fprintf(stderr,"photoproc fail!!! pid=%d\n",m_ProcessID);
		return perc;
	}

	count_t nrcpu  = pSys->cpu.nrcpu;
	count_t availcpu = (SysCur.cpu.all.stime +
			SysCur.cpu.all.utime +
			SysCur.cpu.all.ntime +
			SysCur.cpu.all.itime +
			SysCur.cpu.all.wtime +
			SysCur.cpu.all.Itime +
			SysCur.cpu.all.Stime +
			SysCur.cpu.all.steal +
			SysCur.cpu.all.guest) -
						(pSys->cpu.all.stime +
						pSys->cpu.all.utime +
						pSys->cpu.all.ntime +
						pSys->cpu.all.itime +
						pSys->cpu.all.wtime +
						pSys->cpu.all.Itime +
						pSys->cpu.all.Stime +
						pSys->cpu.all.steal +
						pSys->cpu.all.guest);
	count_t procTime = (m_c_stat.cpu.stime+m_c_stat.cpu.utime) - (m_p_stat.cpu.stime+m_p_stat.cpu.utime);
	if(procTime < 0)
	{
		procTime = 0;
		//perror("!!!!!!!!!!!!!!!!!!!!!");
	}

	if (availcpu)
	{
			perc = (double)(procTime) *
							100.0 /
							(availcpu / nrcpu);

			if (perc > 100.0 * nrcpu)
					perc = 100.0 * nrcpu;

			if (perc > 100.0 * m_c_stat.gen.nthr)
					perc = 100.0 * m_c_stat.gen.nthr;
	}
	return perc;
}


count_t GetProcessInfo::get_mem_usage()
{
	return m_c_stat.mem.rmem;
}

std::string GetProcessInfo::getProcessStartimeByPid( int pid )
{
	time_t startime;
	tm tm_startime;
	char 		szPath[512];
	sprintf(szPath,"/proc/%d/cmdline",pid);
	startime = getProcessStartime(szPath);
	localtime_r(&startime,&tm_startime);
	strftime(szPath,512,"%Y-%m-%d %H:%M:%S",&tm_startime);
	return string(szPath);
}
#endif
