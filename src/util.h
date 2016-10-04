#include <time.h>
#include <sys/time.h>


#ifdef __MACH__
    #include <mach/clock.h>
    #include <mach/mach.h>
#endif


void current_utc_time(struct timespec *ts);
long fastclock(void);

