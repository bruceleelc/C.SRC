#ifndef LNXPROCDEF_h__
#define LNXPROCDEF_h__

#ifdef linux

#include<stdio.h>
#include<sys/types.h>

#undef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#undef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))

/*
** bit-values for supportflags
*/
#define	ACCTACTIVE	0x00000001
#define	PATCHSTAT	0x00000002
#define	IOSTAT		0x00000004
#define	PATCHACCT	0x00000008

#define	EQ		0
#define SECSDAY		86400
#define RAWNAMESZ	256

#define	MAX32BITVAL	0x100000000LL


#define	IO_READ		"read_bytes:"
#define	IO_WRITE	"write_bytes:"
#define	IO_CWRITE	"cancelled_write_bytes:"

/*
** memory-size formatting possibilities
*/
#define	ANYFORMAT	0
#define	KBFORMAT	1
#define	MBFORMAT	2
#define	GBFORMAT	3
#define	TBFORMAT	4
#define	OVFORMAT	9

#define	PNAMLEN		15
#define	CMDLEN		255
#define	MAXCPU		2048


#define	SCANSTAT 	"%c   %d   %*d  %*d  %*d  %*d  "	\
			"%*d  %lld %*d  %lld %*d  %lld "	\
			"%lld %*d  %*d  %d   %d   %*d  "	\
			"%*d  %ld %lld %lld %*d  %*d  "	\
			"%*d  %*d  %*d  %*d  %*d  %*d  " 	\
			"%*d  %*d  %*d  %*d  %*d  %*d  "	\
			"%d   %d   %d "

/* ATOP-extension line of /proc/pid/stat */
#define ATOPSTAT	"%lld %llu %lld %llu %lld %llu %lld %llu "	\
			"%lld %llu %lld %llu %lld %lld"



typedef	long long	count_t;


/************************************************************************/

struct freqcnt {
        count_t maxfreq;/* frequency in MHz                    */
        count_t cnt;    /* number of clock ticks times state   */
        count_t ticks;  /* number of total clock ticks         */
                        /* if zero, cnt is actul freq          */
};

struct percpu {
	int		cpunr;
	count_t		stime;	/* system  time in clock ticks		*/
	count_t		utime;	/* user    time in clock ticks		*/
	count_t		ntime;	/* nice    time in clock ticks		*/
	count_t		itime;	/* idle    time in clock ticks		*/
	count_t		wtime;	/* iowait  time in clock ticks		*/
	count_t		Itime;	/* irq     time in clock ticks		*/
	count_t		Stime;	/* softirq time in clock ticks		*/
	count_t		steal;	/* steal   time in clock ticks		*/
	count_t		guest;	/* guest   time in clock ticks		*/
        struct freqcnt	freqcnt;/* frequency scaling info  		*/
	count_t		cfuture[1];	/* reserved for future use	*/
};

struct	cpustat {
	count_t	nrcpu;	/* number of cpu's 			*/
	count_t	devint;	/* number of device interrupts 		*/
	count_t	csw;	/* number of context switches		*/
	count_t	nprocs;	/* number of processes started          */
	float	lavg1;	/* load average last    minute          */
	float	lavg5;	/* load average last  5 minutes         */
	float	lavg15;	/* load average last 15 minutes         */
	count_t	cfuture[4];	/* reserved for future use	*/

	struct percpu   all;
	struct percpu   cpu[MAXCPU];
};

/************************************************************************/

struct	memstat {
	count_t	physmem;	/* number of physical pages 	*/
	count_t	freemem;	/* number of free     pages	*/
	count_t	buffermem;	/* number of buffer   pages	*/
	count_t	slabmem;	/* number of slab     pages	*/
	count_t	cachemem;	/* number of cache    pages	*/
	count_t	cachedrt;	/* number of cache    pages (dirty)	*/

	count_t	totswap;	/* number of pages in swap	*/
	count_t	freeswap;	/* number of free swap pages	*/

	count_t	pgscans;	/* number of page scans		*/
	count_t	pgsteal;	/* number of page steals	*/
	count_t	allocstall;	/* try to free pages forced	*/
	count_t	swouts;		/* number of pages swapped out	*/
	count_t	swins;		/* number of pages swapped in	*/

	count_t	commitlim;	/* commit limit in pages	*/
	count_t	committed;	/* number of reserved pages	*/
	count_t	cfuture[4];	/* reserved for future use	*/
};

/************************************************************************/

struct	sstat {
	struct cpustat	cpu;
	struct memstat	mem;
	/*
	struct netstat	net;
	struct intfstat	intf;
	struct dskstat  dsk;
	*/

};

/*
** structure containing only relevant process-info extracted
** from kernel's process-administration
*/
struct tstat {
	/* GENERAL TASK INFO 					*/
	struct gen {
		int	tgid;		/* threadgroup identification 	*/
		int	pid;		/* process identification 	*/
		int	ppid;           /* parent process identification*/
		int	ruid;		/* real  user  identification 	*/
		int	euid;		/* eff.  user  identification 	*/
		int	suid;		/* saved user  identification 	*/
		int	fsuid;		/* fs    user  identification 	*/
		int	rgid;		/* real  group identification 	*/
		int	egid;		/* eff.  group identification 	*/
		int	sgid;		/* saved group identification 	*/
		int	fsgid;		/* fs    group identification 	*/
		int	nthr;		/* number of threads in tgroup 	*/
		char	name[PNAMLEN+1];/* process name string       	*/
		char 	isproc;		/* boolean: process level?      */
		char 	state;		/* process state ('E' = exited)	*/
		int	excode;		/* process exit status		*/
		time_t 	btime;		/* process start time (epoch)	*/
		time_t 	elaps;		/* process elaps time (hertz)	*/
		char	cmdline[CMDLEN+1];/* command-line string       	*/
		int	nthrslpi;	/* # threads in state 'S'       */
		int	nthrslpu;	/* # threads in state 'D'       */
		int	nthrrun;	/* # threads in state 'R'       */
		int	ifuture[4];     /* reserved                     */
	} gen;

	/* CPU STATISTICS						*/
	struct cpu {
		count_t	utime;		/* time user   text (ticks) 	*/
		count_t	stime;		/* time system text (ticks) 	*/
		int	nice;		/* nice value                   */
		int	prio;		/* priority                     */
		int	rtprio;		/* realtime priority            */
		int	policy;		/* scheduling policy            */
		int	curcpu;		/* current processor            */
		int	sleepavg;       /* sleep average percentage     */
		int	ifuture[4];	/* reserved for future use	*/
		count_t	cfuture[4];	/* reserved for future use	*/
	} cpu;

	/* DISK STATISTICS						*/
	struct dsk {
		count_t	rio;		/* number of read requests 	*/
		count_t	rsz;		/* cumulative # sectors read	*/
		count_t	wio;		/* number of write requests 	*/
		count_t	wsz;		/* cumulative # sectors written	*/
		count_t	cwsz;		/* cumulative # written sectors */
					/* being cancelled              */
		count_t	cfuture[4];	/* reserved for future use	*/
	} dsk;

	/* MEMORY STATISTICS						*/
	struct mem {
		count_t	minflt;		/* number of page-reclaims 	*/
		count_t	majflt;		/* number of page-faults 	*/
		count_t	vexec;		/* virtmem execfile (Kb)        */
		count_t	vmem;		/* virtual  memory  (Kb)	*/
		count_t	rmem;		/* resident memory  (Kb)	*/
		count_t vgrow;		/* virtual  growth  (Kb)    	*/
		count_t rgrow;		/* resident growth  (Kb)     	*/
		count_t vdata;		/* virtmem data     (Kb)     	*/
		count_t vstack;		/* virtmem stack    (Kb)     	*/
		count_t vlibs;		/* virtmem libexec  (Kb)     	*/
		count_t vswap;		/* swap space used  (Kb)     	*/
	} mem;

	/* NETWORK STATISTICS						*/
	struct net {
		count_t tcpsnd;		/* number of TCP-packets sent	*/
		count_t tcpssz;		/* cumulative size packets sent	*/
		count_t	tcprcv;		/* number of TCP-packets recved	*/
		count_t tcprsz;		/* cumulative size packets rcvd	*/
		count_t	udpsnd;		/* number of UDP-packets sent	*/
		count_t udpssz;		/* cumulative size packets sent	*/
		count_t	udprcv;		/* number of UDP-packets recved	*/
		count_t udprsz;		/* cumulative size packets sent	*/
		count_t	rawsnd;		/* number of raw packets sent	*/
		count_t	rawrcv;		/* number of raw packets recved	*/
		count_t	cfuture[4];	/* reserved for future use	*/
	} net;
};




count_t subcount(count_t newval, count_t oldval);
int 	droprootprivs(void);
void 	regainrootprivs(void);
void 	cleanstop(int);
time_t 	getboot(void);
time_t	getbootlinux(long);
time_t 	file_getTimestamp(const char *object, mode_t type);
time_t 	getProcessUptime(const char *pidfile);
time_t 	getProcessStartime(const char *pidfile);


#endif 	// linux

#endif	//LNXPROCDEF_h_
