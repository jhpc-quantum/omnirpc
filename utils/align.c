#if 0
static char rcsid[] = "$Id: align.c,v 1.1.1.1 2004-11-03 21:01:38 yoshihiro Exp $";
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

#ifdef HAS_LONGLONG
typedef long long int longlongint;
#endif /* HAS_LONGLONG */
#ifdef HAS_LONGDOUBLE
typedef long double longdouble;
#endif /* HAS_LONGDOUBLE */
#ifndef CHECK_TYPE
#define CHECK_TYPE	double
#endif

typedef struct {
    char a0;
    CHECK_TYPE c;
} checkStruct;


static int
maxAlign(addr)
     char *addr;
{
    unsigned long int val = (unsigned long int)addr;
    int ret = 1;

    if (val % sizeof(char) == 0) {
	ret = sizeof(char);
    }
    if (val % sizeof(short) == 0) {
	ret = sizeof(short);
    }
    if (val % sizeof(int) == 0) {
	ret = sizeof(int);
    }
    if (val % sizeof(long) == 0) {
	ret = sizeof(long);
    }
#ifdef HAS_LONGLONG
    if (val % sizeof(longlongint) == 0) {
	ret = sizeof(longlongint);
    }
#endif /* HAS_LONGLONG */
    if (val % sizeof(float) == 0) {
	ret = sizeof(float);
    }
    if (val % sizeof(double) == 0) {
	ret = sizeof(double);
    }
#ifdef HAS_LONGDOUBLE
    if (val % sizeof(longdouble) == 0) {
	ret = sizeof(longdouble);
    }
#endif /* HAS_LONGDOUBLE */

    return ret;
}


int
main(argc, argv)
     int argc;
     char *argv[];
{
    checkStruct *s;
    int align = -1;
    int mxAlign;

    s = (checkStruct *)malloc(sizeof(checkStruct));

    mxAlign = maxAlign((char *)&(s->c));
    if (mxAlign == sizeof(CHECK_TYPE)) {
	align = sizeof(CHECK_TYPE);
    } else if (mxAlign > sizeof(CHECK_TYPE)) {
	int offset = (char *)&(s->c) - (char *)s;
	if (offset <= sizeof(CHECK_TYPE)) {
	    align = offset;
	} else {
	    align = sizeof(CHECK_TYPE);
	}
    } else {
	align = mxAlign;
    }

    printf("%d\n", align);
    return 0;
}
