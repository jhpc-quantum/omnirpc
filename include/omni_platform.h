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

#undef _ANSI_ARGS_
#undef CONST

#if ((defined(__STDC__) || defined(SABER)) && !defined(NO_PROTOTYPE)) || defined(__cplusplus) || defined(USE_PROTOTYPE) || defined(__USE_FIXED_PROTOTYPES__)
#define _USING_PROTOTYPES_ 1
#define _ANSI_ARGS_(x) x
#define CONST const
#else
#define _ANSI_ARGS_(x) ()
#define CONST /**/
#endif /* __STDC__ ||.... */

#include <stdio.h>

#ifndef NO_STRING_H
#include <string.h>
#endif /* !NO_STRING_H */

#ifndef NO_UNISTD_H
#include <unistd.h>
#endif /* !NO_UNISTD_H */

#ifndef NO_LIMITS_H
#include <limits.h>
#endif /* !NO_LIMITS_H */

#ifndef NO_STDLIB_H
#include <stdlib.h>
#endif /* !NO_STDLIB_H */

#include <ctype.h>

#include <errno.h>

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */

#ifndef NO_NETDB_H
#include <netdb.h>
#endif /* !NO_NETDB_H */

#ifndef NO_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* !NO_SYS_SOCKET_H */

#ifndef NO_NETINET_IN_H
#include <netinet/in.h>
#endif /* !NO_NETINET_IN_H */

#ifndef NO_ARPA_INET_H
#include <arpa/inet.h>
#endif /* !NO_ARPA_INET_H */

#ifndef NO_RESOURCE_H
#include <sys/resource.h>
#endif /* !NO_RESOURCE_H */

#ifdef GETTOD_NOT_DECLARED
extern int      gettimeofday _ANSI_ARGS_((struct timeval *tp, struct timezone *tzp));
#endif /* GETTOD_NOT_DECLARED */

#ifdef HAVE_BSDGETTIMEOFDAY
#define gettimeofday(X, Y) BSDgettimeofday((X), (Y))
#endif /* HAVE_BSDGETTIMEOFDAY */

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif /* HAVE_LOCALE_H */

#if defined(__STDC__) || defined(HAVE_STDARG_H)
#   include <stdarg.h>
#   define OMNI_VARARGS(type, name) (type name, ...)
#   define OMNI_VARARGS_DEF(type, name) (type name, ...)
#   define OMNI_VARARGS_START(type, name, list) (va_start(list, name), name)
#else
#   include <varargs.h>
#   ifdef __cplusplus
#       define OMNI_VARARGS(type, name) (type name, ...)
#       define OMNI_VARARGS_DEF(type, name) (type va_alist, ...)
#   else
#       define OMNI_VARARGS(type, name) ()
#       define OMNI_VARARGS_DEF(type, name) (va_alist)
#   endif
#   define OMNI_VARARGS_START(type, name, list) \
        type name = (va_start(list), va_arg(list, type))
#endif

#ifndef HAVE_STRDUP
extern char	*strdup _ANSI_ARGS_((char *s));
#endif /* HAVE_STRDUP */

/*
 * alloca(). If using gcc, do nothing.
 */
#ifndef __GNUC__
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef OMNI_OS_AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
void *	alloca _ANSI_ARGS_((size_t));
#   endif /* !alloca */
#  endif /* OMNI_OS_AIX */
# endif /* HAVE_ALLOCA_H */
#endif /* !__GNUC__ */

#ifdef bzero
#undef bzero
#endif
#define bzero(p, s)	memset((p), 0, (s))

#ifdef bcopy
#undef bcopy
#endif
#define bcopy(s, d, l)	memcpy((d), (s), (l))

#ifdef HAS_INT16
typedef short _omInt16_t;
typedef unsigned short _omUint16_t;
typedef _omInt16_t _omShort;
typedef _omUint16_t _omUshort;
#endif /* HAS_INT16 */

#ifdef HAS_INT32
typedef int _omInt32_t;
typedef unsigned int _omUint32_t;
typedef _omInt32_t _omInt;
typedef _omUint32_t _omUint;
#endif /* HAS_INT32 */

#ifdef HAS_INT64
typedef long long int _omInt64_t;
typedef unsigned long long int _omUint64_t;
typedef _omInt64_t _omLonglongint;
typedef _omUint64_t _omUlonglongint;
#endif /* HAS_INT64 */

typedef unsigned int _omAddrInt_t;
#endif /* _OMNI_PLATFORM_H */
