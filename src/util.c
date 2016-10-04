
#include "util.h"

/* portable time, as listed in https://gist.github.com/jbenet/1087739  */
void current_utc_time(struct timespec *ts) {
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, ts);
#endif
}


/* a better clock() in milliseconds */
long fastclock(void)
{
	struct timespec t;
	long r;

	current_utc_time (&t);
	r = t.tv_sec * 1000;
	r = r + t.tv_nsec / 1000000;
	return r;
}

int kbhit (void)
{
  	struct timeval tv;
  	fd_set rdfs;

  	tv.tv_sec = 0;
  	tv.tv_usec = 0;

  	FD_ZERO(&rdfs);
  	FD_SET (STDIN_FILENO, &rdfs);

  	select (STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  	return FD_ISSET(STDIN_FILENO, &rdfs);
}

