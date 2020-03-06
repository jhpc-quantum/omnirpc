static char rcsid[] = "$Id: cpi-omrpc.c,v 1.1.1.1 2004-11-03 21:01:22 yoshihiro Exp $";

/* 
 * to calcurate PI using OmniRpc.
 * Refer mpich cpi.c.
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

#include "OmniRpc.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define NPROC 8

double second(void);

int
main(int argc, char *argv[])
{
    int done =0, n ,  i;
    double PI25DT = 3.141592653589793238462643;
    double mypi[NPROC], pi;
    OmniRpcRequest reqs[NPROC];
    double startwtime = 0.0, endwtime;

    OmniRpcInit(&argc,&argv);
    printf("initialize ...\n");

    n = 0;
    while(!done){
      /*
	printf("Enter the number of intervals: (0 quits) ");
	scanf("%d",&n);
      */
      if(n == 0) n = 100; else n =0;
      
      startwtime = second();

      if(n == 0){
	done = 1;
      }else {
	for(i = 0; i < NPROC; i++){
	    reqs[i] = OmniRpcCallAsync("calMypi",i, n, NPROC, &(mypi[i]));
	}
	OmniRpcWaitAll(NPROC,reqs);

	pi = 0;
	for(i = 0; i < NPROC; i++){
	  pi += mypi[i];
	}
          
	printf("pi is approximately %.16f, Error is %.16f\n",
	       pi, fabs(pi - PI25DT));
	endwtime = second();
	printf("wall clock time = %f\n",endwtime-startwtime);
      }
    }
    
    OmniRpcFinalize();

    return 0;
}


#include <sys/time.h>
#include <unistd.h>

double
second(void)
{
  struct timeval tm;
  double t;
  gettimeofday(&tm, NULL);
  t = (double)(tm.tv_sec) + ((double)(tm.tv_usec))/1.0e6;
  return t;
}
