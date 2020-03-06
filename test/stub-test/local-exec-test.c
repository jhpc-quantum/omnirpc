static char rcsid[] = "$Id: local-exec-test.c,v 1.1.1.1 2004-11-03 21:01:38 yoshihiro Exp $";
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

extern void mmul(int n, double *A, double *B, double *C);

#define N 10
double A[N*N],B[N*N],C[N*N],D[N*N];

int
main(int argc, char *argv[])
{
    double a1,a2,a3;
    int i,j,k;

#ifdef not
    extern int omrpc_debug_flag;
    omrpc_debug_flag = 1;
#endif

    a1 = 1.234;
    a2 = sin(a1);
    a3 = 0.0;
    printf("sin(%g)=%g\n",a1,a2);

    OmniRpcExecInit(&argc,&argv);

    OmniRpcExecLocal("./sample.rex","sin",a1,&a3);
    printf("omrpc sin(%g)=%g\n",a1,a3);
    
    k = 0;
    for(i = 0; i < N; i++)
	for(j = 0; j < N; j++){
	    A[N*i+j] = k;
	    B[N*j+i] = k+3;
	    k++;
	}

    mmul(N,A,B,C);

    printf("test mmul ...\n");
    OmniRpcExecLocal("./sample.rex","mmul",N,A,B,D);
    
    for(i = 0; i < N; i++)
	for(j = 0; j < N; j++){
	    if(C[i*N+j] != D[i*N+j]){
		printf("invalid!");
		exit(1);
	    }
	}
    printf("mmul verified!!\n");
    
    printf("test end ...\n");

    OmniRpcExecFinalize();

    return 0;
}

