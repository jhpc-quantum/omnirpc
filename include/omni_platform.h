/* 
 * $Id: omni_platform.h,v 1.1.1.1 2004-11-03 21:01:36 yoshihiro Exp $
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
#ifndef _OMNI_PLATFORM_H
#define _OMNI_PLATFORM_H

#include "omni_config.h"

#ifdef USE_MPI
#include <mpi.h>
#endif /* USE_MPI */

#ifdef USE_XMP
#include <xmp.h>
#endif /* USE_XMP */

#endif /* _OMNI_PLATFORM_H */
