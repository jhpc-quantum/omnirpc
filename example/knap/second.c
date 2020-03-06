#define NULL (void *)0
/*----------------------*/ 
#include <sys/time.h>
#include <unistd.h>

double second()
{
    struct timeval tm;
    double t ;
    gettimeofday(&tm,NULL);
    t = (double) (tm.tv_sec) + ((double) (tm.tv_usec))/1.0e6;
    return t;
}
