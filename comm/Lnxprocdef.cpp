#ifdef linux //just for linux

#include "Lnxprocdef.h"
#include "Tool.h"
#include <error.h>
#include <sys/stat.h>
#include <time.h>

/*
** Generic function to subtract two counters taking into
** account the possibility of overflow of a 32-bit kernel-counter.
*/
count_t subcount(count_t newval, count_t oldval)
{
	if (newval >= oldval)
		return newval - oldval;
	else
		return MAX32BITVAL + newval - oldval;
}


/**
 * Get a object's last modified timestamp.
 * @param object A object to stat
 * @param type Requested object's type
 * @return Max of either st_mtime or st_ctime or
 * FALSE if not found or different type of object
 */
time_t file_getTimestamp(const char *object, mode_t type)
{

  struct stat buf;

  ASSERT(object);

  if(! stat(object, &buf)) {
    if(((type == S_IFREG) && S_ISREG(buf.st_mode)) ||
       ((type == S_IFDIR) && S_ISDIR(buf.st_mode)) ||
       ((type == (S_IFREG|S_IFDIR)) && (S_ISREG(buf.st_mode) ||
					S_ISDIR(buf.st_mode)))
       ) {
      return MAX(buf.st_mtime, buf.st_ctime);
    } else {
      fprintf(stderr,"Invalid object type - %s\n",object);
    }
  }

  return 0;

}

time_t getProcessUptime(const char *pidfile)
{
        time_t ctime;

        ASSERT(pidfile);

        if((ctime= file_getTimestamp(pidfile, S_IFREG)) ) {
                time_t now= time(&now);
                time_t since= now-ctime;
                return since;
        }
        return (time_t)-1;
}

time_t 	getProcessStartime(const char *pidfile)
{
	time_t ctime;

	ASSERT(pidfile);

	if((ctime= file_getTimestamp(pidfile, S_IFREG)) ) {
		return ctime;
	}
	return 0;
}




/*
** drop the root privileges that might be obtained via setuid-bit
**
** this action may only fail with errno EPERM (normal situation when
** atop has not been started with setuid-root privs); when this
** action fails with EAGAIN or ENOMEM, atop should not continue
** without root privs being dropped...
*/
int droprootprivs(void)
{
	if (seteuid( getuid() ) == -1 && errno != EPERM)
		return 0;	/* false */
	else
		return 1;	/* true  */
}


/*
** regain the root privileges that might be dropped earlier
*/
void regainrootprivs(void)
{
	seteuid(0);
}



#endif 	// linux
