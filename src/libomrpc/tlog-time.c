static char rcsid[] = "$Id: tlog-time.c,v 1.1.1.1 2004-11-03 21:01:18 yoshihiro Exp $";
/* 
 * $Release: omnirpc-2.0.1 $
 * $Copyright:
 *  OmniRPC Version 1.0
 *  Copyright (C) 2002-2004 HPCS Laboratory, University of Tsukuba.
 *  
 *  This software is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version
 *  2.1 published by the Free Software Foundation.
 *  
 *  Please check the Copyright and License information in the files named
 *  COPYRIGHT and LICENSE under the top  directory of the OmniRPC Grid PRC 
 *  System release kit.
 *  
 *  
 *  $
 */
/*
 *  TIMER routine
 *
 *  $Id: tlog-time.c,v 1.1.1.1 2004-11-03 21:01:18 yoshihiro Exp $
 */
#include <stddef.h>

#if !defined(__TTIME_GETTIMEOFDAY__) || !defined(__TTIME_MPI_WTIME__)

/**  BEGINNING OF MACHINE-DEPENDENT PART  **/

/***  SUN SOLARIS  ***/
#if defined(OMNI_OS_SOLARIS)
#include <sys/time.h>

#ifdef __GNUC__
#define USE_LL
#endif

#if __STDC__ - 0 == 0 && !defined(_NO_LONGLONG)
#define USE_LL
#endif

double
tlog_timestamp()
{
#ifdef USE_LL
    return ((double)gethrtime()) * .000000001;
#else
    hrtime_t hrT = gethrtime();
    return hrT._d * .000000001;
#endif /* USE_LL */
}

/***  SGI IRIX  ***/
#elif defined(OMNI_OS_IRIX)
#include <time.h>

double
tlog_timestamp()
{
    struct timespec t1;
    clock_gettime(CLOCK_SGI_CYCLE, &t1);
    return (double)t1.tv_sec + (double)t1.tv_nsec * .000000001;
}

/***  gettimeofday(2)  ***/
#else
#include <sys/time.h>

double
tlog_timestamp()
{
    struct timeval t1;
    gettimeofday(&t1, NULL);
    return (double)t1.tv_sec + (double)t1.tv_usec * .000001;
}

#endif

/**  END OF MACHINE-DEPENDENT PART  **/

/**  BEGINNING OF MACHINE-INDEPENDENT PART  **/

#elif defined(__TTIME_GETTIMEOFDAY__)

#include <sys/time.h>

double
tlog_timestamp()
{
    struct timeval t1;
    gettimeofday(&t1, NULL);
    return (double)t1.tv_sec + (double)t1.tv_usec * .000001;
}

#else /** defined(__TTIME_MPI_WTIME__) **/

/*
 *  TIMER routine for MPI
 */

double
tlog_timestamp()
{
    return MPI_Wtime();
}

#endif /** !defined(__TTIME_GETTIMEOFDAY__) || !defined(__TTIME_MPI_WTIME__) **/

/**  END OF MACHINE-INDEPENDENT PART  **/
