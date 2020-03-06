#if 0
static char rcsid[] = "$Id: hasqd.c,v 1.1.1.1 2004-11-03 21:01:39 yoshihiro Exp $";
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
#include "confdefs.h"


int
main(argc, argv)
     int argc;
     char *argv[];
{
    printf("%qd\n", 1);
}
