
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <math.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h> // for usleep()


// Detect OSX
#ifdef __MACH__
    #include <mach/clock.h>
    #include <mach/mach.h>
#endif 

// Detect Raspi
#ifdef __ARM
    #include <linux/ioctl.h>
    #include <asm-generic/ioctl.h>
    #include <asm-generic/termios.h>
#endif




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


/* get sockaddr, IPv4 or IPv6: */
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void msleep(int d)
{
    usleep(d*1000);
}

