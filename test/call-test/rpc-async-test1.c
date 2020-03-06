static char rcsid[] = "$Id: rpc-async-test1.c,v 1.1.1.1 2004-11-03 21:01:37 yoshihiro Exp $";
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
#include <math.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    double a1,a2,a3;
    double r1,r2,r3;
    OmniRpcRequest rq1,rq2,rq3;
    double t,r[10];
    OmniRpcRequest reqs[10];
    int i;

#ifdef not
    extern int omrpc_debug_flag;
    omrpc_debug_flag = 1;
#endif
    a1 = 1.234;
    a2 = 3.45;
    a3 = 4.96;

    OmniRpcInit(&argc,&argv);

    printf("submit 1 ...\n");
    rq1 = OmniRpcCallAsync("sin",a1,&r1);
    printf("submit 2 ...\n");
    rq2 = OmniRpcCallAsync("sin",a2,&r2);
    printf("submit 3 ...\n");
    rq3 = OmniRpcCallAsync("sin",a3,&r3);

    printf("wait 1 ...\n");
    OmniRpcWait(rq1);
    printf("wait 2 ...\n");
    OmniRpcWait(rq2);
    printf("wait 3 ...\n");
    OmniRpcWait(rq3);

    printf("1: %g %g\n",r1,sin(a1));
    if(r1 != sin(a1)) printf("1 is failed\n");
    printf("2: %g %g\n",r2,sin(a2));
    if(r2 != sin(a2)) printf("2 is failed\n");
    printf("3: %g %g\n",r3,sin(a3));
    if(r3 != sin(a3)) printf("3 is failed\n");

    t = 0.0;
    for(i = 0; i < 10; i++){
	reqs[i] = OmniRpcCallAsync("sin",t,r+i);
	t += 0.1;
    }
    OmniRpcWaitAll(10,reqs);

    t = 0.0;
    for(i = 0; i < 10; i++){
	if(r[i] != sin(t)) 
	    printf("r[%d] = %g (%g), not verified\n",i,r[i],sin(t));
	t += 0.1;
    }

    printf("test end ...\n");
    
    OmniRpcFinalize();

    return 0;
}


