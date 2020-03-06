static char rcsid[] = "$Id: rpc-call-test3.c,v 1.1.1.1 2004-11-03 21:01:37 yoshihiro Exp $";
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
    int i, x, y, r;

    OmniRpcInit(&argc,&argv);

    printf("initialize ...\n");
    OmniRpcModuleInit("init_sample",10);

    for(i = 0; i < 10; i++){
	OmniRpcCall("test",2,&x,&y);
	printf("x = %d, y = %d\n",x,y);
    }

    for(i = 0; i < 10; i++){
      printf("call work i=%d\n",i);
      OmniRpcCall("work",i,&r);
    }
    
    printf("test end ...\n");
    
    OmniRpcFinalize();

    return 0;
}


