#include <time.h>
#include <sys/time.h>
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


#ifdef __MACH__
    #include <mach/clock.h>
    #include <mach/mach.h>
#endif


void current_utc_time(struct timespec *ts);
long fastclock(void);
int kbhit (void);
void *get_in_addr(struct sockaddr *sa);

