#if 0
static char rcsid[] = "$Id: size.c,v 1.1.1.1 2004-11-03 21:01:39 yoshihiro Exp $";
#endif
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *
chkIntSize(sz)
     int sz;
{
    if (sizeof(char) == sz) {
	return "char";
    } else if (sizeof(int) == sz) {
	return "int";
    } else if (sizeof(short) == sz) {
	return "short";
#ifdef HAS_LONGLONG
    } else if (sizeof(long long int) == sz) {
	return "long long int";
#endif
    } else if (sizeof(long int) == sz) {
	return "long int";
    } else {
	return "unknown";
    }
}


int
main(argc, argv)
     int argc;
     char *argv[];
{
    int sz;

    if (argc < 2) {
	return 1;
    }
    sz = atoi(argv[1]) / 8;

    printf("%s\n", chkIntSize(sz));
    return 0;
}
