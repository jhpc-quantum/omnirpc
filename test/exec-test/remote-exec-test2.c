static char rcsid[] = "$Id: remote-exec-test2.c,v 1.1.1.1 2004-11-03 21:01:38 yoshihiro Exp $";
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
    char *dir;
    char buf[128];
    OmniRpcExecHandle handle;

    OmniRpcExecInit(&argc,&argv);

    if(argc != 3){
	printf("%s hostname stub_dir\n",argv[0]);
	exit(1);
    }
    host = argv[1];
    dir = argv[2];
    printf("host: '%s'\n",host);
    printf("stub directory: '%s'\n",dir);

    a1 = 1.234;
    a2 = sin(a1);
    a3 = 0.0;
    printf("sin(%g)=%g\n",a1,a2);

    strcpy(buf,dir);
    strcat(buf,"/");
    strcat(buf,"sample.rex");
    handle = OmniRpcExecOnHost(host,buf);

    OmniRpcExecCall(handle,"sin",a1,&a3);

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

    OmniRpcExecCall(handle,"mmul",N,A,B,D);
    
    for(i = 0; i < N; i++)
	for(j = 0; j < N; j++){
	    if(C[i*N+j] != D[i*N+j]){
		printf("invalid!");
		exit(1);
	    }
	}
    printf("mmul verified!!\n");
    printf("test end ...\n");
    OmniRpcExecTerminate(handle);

    OmniRpcExecFinalize();

    return 0;
}


