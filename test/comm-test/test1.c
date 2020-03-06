static char rcsid[] = "$Id: test1.c,v 1.1.1.1 2004-11-03 21:01:37 yoshihiro Exp $";
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
#include "OmniRpc.h"
#include <stdio.h>
#include <string.h>

#define N 1000
int in[N],out[N];

double second();

int
main(int argc, char *argv[])
{
  char *s;
  int r;
  int i;
  double t1,t2;
#ifdef not
  extern int omrpc_debug_flag;
  omrpc_debug_flag = 1;
#endif
  printf("test1 ...\n");
  OmniRpcInit(&argc,&argv);

  OmniRpcCall("hostname",&s,&r);
  printf("hostname ='%s' r=%d\n",s,r);
    
  t1 = second();
  OmniRpcCall("echo",1,in,1,out);
  t2 = second();
  printf("1 time=%g\n",t2-t1);

  t1 = second();
  OmniRpcCall("echo",10,in,10,out);
  t2 = second();
  printf("10 time=%g\n",t2-t1);

  t1 = second();
  OmniRpcCall("echo",100,in,100,out);
  t2 = second();
  printf("100 time=%g\n",t2-t1);
  
  t1 = second();
  OmniRpcCall("echo",1000,in,1000,out);
  t2 = second();
  printf("1000 time=%g\n",t2-t1);

  printf("test1 end ...\n");
    
  OmniRpcFinalize();

  return 0;
}

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
