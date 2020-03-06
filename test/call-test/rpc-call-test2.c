static char rcsid[] = "$Id: rpc-call-test2.c,v 1.1.1.1 2004-11-03 21:01:37 yoshihiro Exp $";
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
#include <stdlib.h>

extern void mmul(int n, double *A, double *B, double *C);

#define N 10
double A[N*N],B[N*N],C[N*N],D[N*N];

int
main(int argc, char *argv[])
{
    double a1,a2,a3;
    int i,j,k;
    char *host;
    void *handle;
#ifdef not
    extern int omrpc_debug_flag;
    omrpc_debug_flag = 1;
#endif
    OmniRpcInit(&argc,&argv);

    if(argc != 2){
	printf("%s hostname\n",argv[0]);
	exit(1);
    }
    host = argv[1];

    printf("host: '%s'\n",host);

    a1 = 1.234;
    a2 = sin(a1);
    a3 = 0.0;
    printf("sin(%g)=%g\n",a1,a2);

    handle = OmniRpcCreateHandle(host,"sample");
    if(handle == NULL){
	printf("cannot create handle\n");
	exit(1);
    }

    OmniRpcCallByHandle(handle,"sin",a1,&a3);
    printf("grpc sin(%g)=%g\n",a1,a3);
    
    k = 0;
    for(i = 0; i < N; i++)
	for(j = 0; j < N; j++){
	    A[N*i+j] = k;
	    B[N*j+i] = k+3;
	    k++;
	}

    printf("test mmul...\n");

    mmul(N,A,B,C);

    OmniRpcCallByHandle(handle,"mmul",N,A,B,D);
    
    for(i = 0; i < N; i++)
	for(j = 0; j < N; j++){
	    if(C[i*N+j] != D[i*N+j]){
		printf("invalid!");
		exit(1);
	    }
	}
    printf("mmul verified!!\n");

    OmniRpcDestroyHandle(handle);

    printf("test end ...\n");
    
    OmniRpcFinalize();

    return 0;
}


