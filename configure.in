dnl $Id: configure.in,v 1.1.1.1 2004-11-03 21:01:01 yoshihiro Exp $
dnl $Release: omnirpc-2.2.2 $
dnl $Copyright:
dnl  OmniRPC Version 2.0
dnl  Copyright (C) 2002-2004 HPCS Laboratory, University of Tsukuba.
dnl  
dnl  This software is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU Lesser General Public License version
dnl  2.1 published by the Free Software Foundation.
dnl  
dnl  Please check the Copyright and License information in the files named
dnl  COPYRIGHT and LICENSE under the top  directory of the OmniRPC Grid PRC 
dnl  System release kit.
dnl  
dnl  
dnl  $
dnl	This file is an input file used by the GNU "autoconf" program to
dnl	generate the file "configure", which is run during Omni installation
dnl	to configure the system for the local environment.

# --------------------------------------------------------------------
# Let the initiation begin....

#AC_INIT(include/omni_platform.h)
# --------------------------------------------------------------------
AC_INIT()

# --------------------------------------------------------------------
# Set Omni version

#omniVersion=1.2s
#AC_SUBST(omniVersion)
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# prefix directory check and modify

if test "${prefix}" = "NONE"; then
    prefix=/usr/local
fi

orgPrefix=${prefix}
AC_SUBST(orgPrefix)

t=`echo ${prefix} | egrep '^/.*$'`
if test "x${t}" = "x${prefix}"; then
    prefix="\$(DESTDIR)${prefix}"
else
    prefix="\$(DESTDIR)/${prefix}"
fi
unset t

if test "${exec_prefix}" = "NONE"; then
    orgExec_prefix=${orgPrefix}
    exec_prefix='${prefix}'
else
    orgExec_prefix=${exec_prefix}
    t=`echo ${exec_prefix} | egrep '^/.*$'`
    if test "x${t}" = "x${exec_prefix}"; then
	exec_prefix="\$(DESTDIR)${exec_prefix}"
    else
	exec_prefix="\$(DESTDIR)/${exec_prefix}"
    fi
fi
AC_SUBST(orgExec_prefix)
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# First of all, check on what system we are running.

HOSTARCH=`./utils/config.guess | grep -v Linking`
CPU=`echo $HOSTARCH | awk -F- '{ print $1 }' | sed 's/\./_/g'`
OS=`echo $HOSTARCH | awk -F- '{ print $3 }' | sed 's/\./_/g'`

case $CPU in
    i*86)
        CPU=i386;;
    alpha*)
	CPU=alpha;;
esac

case $OS in
    sunos*)
        OS=sunos;;
    solaris*)
        OS=solaris;;
    freebsd*)
	OS=freebsd;;
    irix*)
	OS=irix;;
    aix*)
	OS=aix;;
esac

AC_SUBST(OS)
AC_SUBST(CPU)
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# Next, check what CC will be used, and set compile flags

ccType="unknown"

isSunCC=no
sunCCPre6=0
isMipsCC=no
isAIXCC=no
isGcc=no

mustCFlags=""
mustCppFlags=""

optCFlags=""
warnCFlags=""
debCFlags=""

userOptCFlags=""
userDebCFlags=""

debCppFlags="-DNDEBUG"
userCppFlags=""

ccCom=""
AC_ARG_WITH(cc,
    [  --with-cc=CC            specify C compiler to use ("cc" default)],
    [ccCom=$withval], [ccCom=""])

AC_ARG_WITH(opt,
    [  --with-opt=OPT          specify C compiler options for optimization],
    [userOptCFlags=$withval], [userOptCFlags=""])

AC_ARG_WITH(debug,
    [  --with-debug=OPT        specify C compiler options for debuggable executaable file creation],
    [userDebCFlags=$withval], [userDebCFlags=""])

AC_ARG_WITH(cppflag,
    [  --with-cppflag=OPT      specify C preprocessor options],
    [userCppFlags=$withval], [userCppFlags=""])


oCC=${CC}
oCFLAGS=${CFLAGS}
progCC=""
AC_ARG_ENABLE(gcc,
    [  --enable-gcc            allow use of gcc if available],
    [exc_ok=$enableval], [exc_ok=no])
if test "$exc_ok" = "yes"; then
    AC_PROG_CC
    progCC=${CC}
fi
CFLAGS=${oCFLAGS}
CC=${oCC}

if test "x${ccCom}" != "x"; then
    CC=${ccCom}
elif test "x${progCC}" != "x"; then
    CC=${progCC}
fi

if test "x${CC}" = "x"; then
   CC=cc
fi

AC_ARG_ENABLE(debug,
    [  --enable-debug          enable generate executable with debug symbol (false default).],
    [doDebug=$enableval], [doDebug=no])
if test "x${doDebug}" = "xyes"; then
    debCFlags="-g"
    debCppFlags="-DDEBUG"
fi

# Check it's GCC by command name
cCom=`echo ${CC} | awk '{ print $1 }'`
if test "x${GCC}" = "xyes" -o "`basename ${cCom}`" = "gcc" ; then
    isGcc=yes
fi
if test "x${isGcc}" = "xno"; then
    # Check it's GCC by command line option
    tmp=/tmp/.OmniConfCC
    ${cCom} -v > $tmp 2>&1
    if test $? -eq 0; then
	chk=`tail -1 $tmp | awk '{ print $3 }'`
	case $chk in
	    egcs*|2.*)
		isGcc=yes;;
	    *)
		isGcc=no;;
	esac
    fi
    rm -f $tmp
fi

# OK now it must be decided gcc or not.
if test "x${isGcc}" = "xno"; then
    case $OS in
	solaris*)
	    tmp=/tmp/.OmniConf.$$
	    ${CC} -V > $tmp 2>&1
	    if test $CPU = i386; then
		cStr='cc: PC'
	    else
		cStr='WorkShop'
	    fi
	    x=`grep "$cStr" $tmp`
	    if test "X${x}" != "X"; then
		isSunCC=yes
		case $CPU in
		    sparc*)
			cStr='WorkShop Compilers'
			x=`grep "$cStr" $tmp`
			if test "X${x}" != "X"; then
			    sunCCPre6=1
			fi;;
		esac
	    fi
	    rm -f $tmp
	    ;;
	irix*)
	    tmp=/tmp/.OmniConf.$$
	    ${CC} -version > $tmp 2>&1
	    x=`grep 'MIPSpro' $tmp`
	    if test "X${x}" != "X"; then
		isMipsCC=yes
	    fi
	    rm -f $tmp
	    ;;
	aix*)
	    # XXXXX FIXEME: Are there any other compilers running on AIX???
	    # Anyway asume it's XLC.
	    isAIXCC=yes
	    ;;
    esac
fi

# GCC
if test "x${isGcc}" = "xyes"; then
    # for old gcc/systems ...
    mustCppFlags="-D__USE_FIXED_PROTOTYPES__"
    warnCFlags="-Wall"
    optCFlags="-O2"
    CPP="$CC -E"
    ccType=gcc
fi

# Sun CC
if test "X${isSunCC}" = "Xyes"; then
    mustCppFlags="-D__EXTENSIONS__"
    if test $sunCCPre6 -eq 0; then
	mustCppFlags="${mustCppFlags} -DSUN_WS_CC_6"
    fi
    mustCFlags="-v"
    debCFlags="${debCFlags} -xs"
    if test $CPU = i386; then
	oLvl=4
    else
	oLvl=5
    fi
    optCFlags="-fast -xO${oLvl}"
    ccType=SunCC
fi

# Mips CC
if test "X${isMipsCC}" = "Xyes"; then
    optCFlags="-O3"
    ccType=MipsCC
fi

# AIX XLC
if test "X${isAIXCC}" = "Xyes"; then
    CC=cc_r
    mustCppFlags="-DUSE_PROTOTYPE"
    optCFlags="-O3 -qstrict"
    ccType=AIXCC
fi

# Generate cpu/os specifier
changequote(<<, >>)dnl
CPUDEF=-DOMNI_CPU_`echo $CPU | tr '[a-z]' '[A-Z]'`
OSDEF=-DOMNI_OS_`echo $OS | tr '[a-z]' '[A-Z]'`
changequote([, ])dnl

# MT safe CPP flags
MTSafeCppFlags=""
case ${OS} in
    irix)
	MTSafeCppFlags="-D_SGI_MP_SOURCE -D_SGI_REENTRANT_FUNCTIONS";;
    aix)
	MTSafeCppFlags="-D_THREAD_SAFE";;
    solaris|linux|freebsd)
	MTSafeCppFlags="-D_REENTRANT";;
    *)
	true;;
esac
mustCppFlags="${mustCppFlags} ${MTSafeCppFlags}"

# Finally, emit.
AC_SUBST(CC)
AC_SUBST(ccType)

CPPFLAGS="${CPPFLAGS} ${CPUDEF} ${OSDEF} ${debCppFlags} ${userCppFlags} ${mustCppFlags}"
AC_SUBST(CPPFLAGS)

if test "x${doDebug}" != "xyes"; then
    if test "x${userOptCFlags}" = "x"; then
	optOpt=${optCFlags}
    else
 	optOpt=${userOptCFlags}
    fi
else
    if test "x${userDebCFlags}" = "x"; then
	optOpt=${debCFlags}
    else
 	optOpt=${userDebCFlags}
    fi
fi
CFLAGS="${CFLAGS} ${warnCFlags} ${optOpt} ${mustCFlags}"
AC_SUBST(CFLAGS)
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# Generic config from here.

#--------------------------------------------------------------------
#	alloca check.
#--------------------------------------------------------------------
AC_FUNC_ALLOCA

#--------------------------------------------------------------------
#	Supply substitutes for missing POSIX header files.  Special
#	notes:
#	    - stdlib.h doesn't define strtol, strtoul, or
#	      strtod insome versions of SunOS
#	    - some versions of string.h don't declare procedures such
#	      as strstr
#--------------------------------------------------------------------
AC_CHECK_HEADER(errno.h, , AC_DEFINE(NO_ERRNO_H))
AC_CHECK_HEADER(limits.h, , AC_DEFINE(NO_LIMITS_H))
AC_CHECK_HEADER(stdlib.h, , AC_DEFINE(NO_STDLIB_H))
AC_CHECK_HEADER(string.h, , AC_DEFINE(NO_STRING_H))
AC_CHECK_HEADER(strings.h, , AC_DEFINE(NO_STRINGS_H))
AC_CHECK_HEADER(unistd.h, , AC_DEFINE(NO_UNISTD_H))
AC_CHECK_HEADER(stdarg.h, AC_DEFINE(HAVE_STDARG_H),)

AC_CHECK_HEADER(locale.h, haveLocale=yes, haveLocale=no)
if test "X${haveLocale}" = "Xyes"; then
	AC_DEFINE(HAVE_LOCALE_H)
	AC_EGREP_HEADER(setlocale, locale.h, AC_DEFINE(HAVE_SETLOCALE),)
fi

AC_CHECK_HEADER(ieeefp.h, , AC_DEFINE(NO_IEEEFP_H))
AC_CHECK_HEADER(float.h, , AC_DEFINE(NO_FLOAT_H)) 
AC_CHECK_HEADER(netdb.h, , AC_DEFINE(NO_NETDB_H))
AC_CHECK_HEADER(sys/socket.h, , AC_DEFINE(NO_SYS_SOCKET_H))
AC_CHECK_HEADER(netinet/in.h, , AC_DEFINE(NO_NETINET_IN_H))
AC_CHECK_HEADER(arpa/inet.h, , AC_DEFINE(NO_ARPA_INET_H))
AC_CHECK_HEADER(sys/resource.h, , AC_DEFINE(NO_RESOURCE_H))

AC_EGREP_HEADER(strdup, string.h, AC_DEFINE(HAVE_STRDUP),)

#------------------------------------------------------------------------------
#       Find out all about time handling differences.
#------------------------------------------------------------------------------
AC_CHECK_HEADERS(sys/time.h)
AC_HEADER_TIME

#--------------------------------------------------------------------
#	Check for various typedefs and provide substitutes if
#	they don't exist.
#--------------------------------------------------------------------
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

#--------------------------------------------------------------------
#	endian check
#--------------------------------------------------------------------
AC_C_BIGENDIAN

#--------------------------------------------------------------------
#	The code below deals with several issues related to gettimeofday:
#	1. Some systems don't provide a gettimeofday function at all
#	   (set NO_GETTOD if this is the case).
#	2. SGI systems don't use the BSD form of the gettimeofday function,
#	   but they have a BSDgettimeofday function that can be used instead.
#	3. See if gettimeofday is declared in the <sys/time.h> header file.
#	   if not, set the GETTOD_NOT_DECLARED flag so that tclPort.h can
#	   declare it.
#--------------------------------------------------------------------
AC_CHECK_FUNC(BSDgettimeofday, AC_DEFINE(HAVE_BSDGETTIMEOFDAY),
	AC_CHECK_FUNC(gettimeofday, , AC_DEFINE(NO_GETTOD)))
AC_MSG_CHECKING([for gettimeofday declaration])
AC_EGREP_HEADER(gettimeofday, sys/time.h, AC_MSG_RESULT(present), [
    AC_MSG_RESULT(missing)
    AC_DEFINE(GETTOD_NOT_DECLARED)
])


# check system libraries
MATH_LIBS="-lm"
#--------------------------------------------------------------------
#       On AIX systems, libbsd.a has to be linked in to support
#       non-blocking file IO.  This library has to be linked in after
#       the MATH_LIBS or it breaks the pow() function.  The way to
#       insure proper sequencing, is to add it to the tail of MATH_LIBS.
#        This library also supplies gettimeofday.
#--------------------------------------------------------------------
libbsd=no
if test "`uname -s`" = "AIX" ; then
    AC_CHECK_LIB(bsd, gettimeofday, libbsd=yes)
    if test $libbsd = yes; then
	MATH_LIBS="$MATH_LIBS -lbsd"
    fi
fi
AC_SUBST(MATH_LIBS)


SOCK_LIBS=""
#--------------------------------------------------------------------
#       Check for the existence of the -lsocket and -lnsl libraries.
#       The order here is important, so that they end up in the right
#       order in the command line generated by make.  Here are some
#       special considerations:
#       1. Use "connect" and "accept" to check for -lsocket, and
#          "gethostbyname" to check for -lnsl.
#       2. Use each function name only once:  can't redo a check because
#          autoconf caches the results of the last check and won't redo it.
#       3. Use -lnsl and -lsocket only if they supply procedures that
#          aren't already present in the normal libraries.  This is because
#          IRIX 5.2 has libraries, but they aren't needed and they're
#          bogus:  they goof up name resolution if used.
#       4. On some SVR4 systems, can't use -lsocket without -lnsl too.
#          To get around this problem, check for both libraries together
#          if -lsocket doesn't work by itself.
#--------------------------------------------------------------------
oLIBS=$LIBS
LIBS=""
omni_checkBoth=0
AC_CHECK_FUNC(connect, omni_checkSocket=0, omni_checkSocket=1)
if test "$omni_checkSocket" = 1; then
    AC_CHECK_LIB(socket, main, LIBS="$LIBS -lsocket", omni_checkBoth=1)
fi
if test "$omni_checkBoth" = 1; then
    omni_oldLibs=$LIBS
    LIBS="$LIBS -lsocket -lnsl"
    AC_CHECK_FUNC(accept, omni_checkNsl=0, [LIBS=$omni_oldLibs])
fi
AC_CHECK_FUNC(gethostbyname, , AC_CHECK_LIB(nsl, main, [LIBS="$LIBS -lnsl"]))
SOCK_LIBS=$LIBS
LIBS=$oLIBS

#--------------------------------------------------------------------
#       Interactive UNIX requires -linet instead of -lsocket, plus it
#       needs net/errno.h to define the socket-related error codes.
#--------------------------------------------------------------------
if test "x${SOCK_LIBS}" = "x"; then
    oLIBS=$LIBS
    AC_CHECK_LIB(inet, main, [LIBS="$LIBS -linet"])
    AC_CHECK_HEADER(net/errno.h, AC_DEFINE(HAVE_NET_ERRNO_H))
    SOCK_LIBS=$LIBS
    LIBS=$oLIBS
fi

AC_SUBST(SOCK_LIBS)

#--------------------------------------------------------------------
#	Check inet_aton() in $SOCK_LIBS
#--------------------------------------------------------------------
omni_haveIAN=0
oLIBS=$LIBS
LIBS="${LIBS} ${SOCK_LIBS}"
AC_CHECK_FUNC(inet_aton, omni_haveIAN=1, omni_haveIAN=0)
if test "$omni_haveIAN" = 1; then
    AC_DEFINE(HAVE_INET_ATON)
fi
LIBS=$oLIBS

# Generic config end here.
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# check has "long long"

hasLL=no
AC_MSG_CHECKING([has long long])
AC_TRY_COMPILE([],[long long int x;], [hasLL=yes], [hasLL=no])
AC_MSG_RESULT($hasLL)
hasLLFlags=""
if test $hasLL = yes; then
    hasLLFlags="-DHAS_LONGLONG"
    AC_DEFINE(HAS_LONGLONG)
fi

hasLD=no
AC_MSG_CHECKING([has long double])
AC_TRY_COMPILE([],[long double x;], [hasLD=yes], [hasLD=no])
AC_MSG_RESULT($hasLD)
hasLDFlags=""
if test $hasLD = yes; then
    hasLDFlags="-DHAS_LONGDOUBLE"
    AC_DEFINE(HAS_LONGDOUBLE)
fi

AC_CHECK_SIZEOF(unsigned short)
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long)
llTyp=''
if test $hasLL = yes; then
    AC_CHECK_SIZEOF(unsigned long long)
    llTyp='longlongint'
fi
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
ldTyp=''
if test $hasLD = yes; then
    AC_CHECK_SIZEOF(long double)
    ldTyp='longdouble'
fi
AC_CHECK_SIZEOF(void *)
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# align

for i in char short int long "$llTyp" float double "$ldTyp"
do
    if test "x${i}" = x; then
	continue
    fi
    getAlign=0
    rm -rf ./align ./align.*
    case $i in
	longlongint)
	    AC_MSG_CHECKING([default align of long long int]);;
	longdouble)
	    AC_MSG_CHECKING([default align of long double]);;
	*)
	    AC_MSG_CHECKING([default align of $i]);;
    esac
    ${CC} ${CPPFLAGS} ${CFLAGS} ${hasLLFlags} ${hasLDFlags} -DCHECK_TYPE=$i ./utils/align.c -o align
    if test -x ./align; then
	getAlign=`./align | grep -v Linking`
    fi
    if test $getAlign -eq 0; then
	AC_MSG_RESULT(can't determine????)
	exit 1
    else
	AC_MSG_RESULT($getAlign)
	case $i in
	    char)
		AC_DEFINE_UNQUOTED(CHAR_ALIGN, $getAlign);;
	    short)
		AC_DEFINE_UNQUOTED(SHORT_ALIGN, $getAlign);;
	    int)
		AC_DEFINE_UNQUOTED(INT_ALIGN, $getAlign);;
	    long)
		AC_DEFINE_UNQUOTED(LONG_ALIGN, $getAlign);;
	    longlongint)
		AC_DEFINE_UNQUOTED(LONGLONG_ALIGN, $getAlign);;
	    float)
		AC_DEFINE_UNQUOTED(FLOAT_ALIGN, $getAlign);;
	    double)
		AC_DEFINE_UNQUOTED(DOUBLE_ALIGN, $getAlign);;
	    longdouble)
		AC_DEFINE_UNQUOTED(LONGDOUBLE_ALIGN, $getAlign);;
	esac
    fi
    rm -rf ./align ./align.*
done
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# i386 double-align

if test ${CPU} = "i386" -a "x${isGcc}" = "xyes"; then
    for i in char short int long "$llTyp" float double "$ldTyp"
    do
	if test "x${i}" = x; then
	    continue
	fi
	getAlign=0
	rm -rf ./align ./align.*
	case $i in
	    longlongint)
		AC_MSG_CHECKING([i386 double-align of long long int]);;
	    longdouble)
		AC_MSG_CHECKING([i386 double-align of long double]);;
	    *)
		AC_MSG_CHECKING([i386 double-align of $i]);;
	esac
	${CC} ${CPPFLAGS} ${CFLAGS} -malign-double ${hasLLFlags} ${hasLDFlags} -DCHECK_TYPE=$i ./utils/align.c -o align
	if test -x ./align; then
	    getAlign=`./align | grep -v Linking`
	fi
	if test $getAlign -eq 0; then
	    AC_MSG_RESULT(can't determine????)
	    exit 1
	else
	    AC_MSG_RESULT($getAlign)
	    case $i in
		char)
		    AC_DEFINE_UNQUOTED(I386_CHAR_ALIGN2, $getAlign);;
		short)
		    AC_DEFINE_UNQUOTED(I386_SHORT_ALIGN2, $getAlign);;
		int)
		    AC_DEFINE_UNQUOTED(I386_INT_ALIGN2, $getAlign);;
		long)
		    AC_DEFINE_UNQUOTED(I386_LONG_ALIGN2, $getAlign);;
		longlongint)
		    AC_DEFINE_UNQUOTED(I386_LONGLONG_ALIGN2, $getAlign);;
		float)
		    AC_DEFINE_UNQUOTED(I386_FLOAT_ALIGN2, $getAlign);;
		double)
		    AC_DEFINE_UNQUOTED(I386_DOUBLE_ALIGN2, $getAlign);;
		longdouble)
		    AC_DEFINE_UNQUOTED(I386_LONGDOUBLE_ALIGN2, $getAlign);;
	    esac
	fi
	rm -rf ./align ./align.*
    done
fi
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# 32/64 bit int type

rm -rf ./szchk ./szchk.*
AC_MSG_RESULT([Creating size check program])
${CC} ${CPPFLAGS} ${CFLAGS} ${hasLLFlags} ./utils/size.c -o szchk
if test ! -x ./szchk; then
    AC_MSG_RESULT([can't create size check program. exit.])
    exit 1
fi

F2C_INT64_FLAGS=""
for i in 16 32 64
do
    AC_MSG_CHECKING([$i bit integer])
    typName=`./szchk $i`
    if test "x${typName}" = "xunknown"; then
	case $i in
	    16)
		TYPE_INT16="short"
		AC_SUBST(TYPE_INT16)
		asump=${TYPE_INT16};;
	    32)
		TYPE_INT32="int"
		AC_SUBST(TYPE_INT32)
		asump=${TYPE_INT32};;
	    64)
		TYPE_INT64="long long int"
		AC_SUBST(TYPE_INT64)
		asump=${TYPE_INT64};;
	esac
	AC_MSG_RESULT([not supported. use $asump])
    else
	AC_MSG_RESULT([$typName])
	case $i in
	    16)
		AC_DEFINE(HAS_INT16)
		TYPE_INT16=${typName}
		AC_SUBST(TYPE_INT16);;
	    32)
		AC_DEFINE(HAS_INT32)
		TYPE_INT32=${typName}
		AC_SUBST(TYPE_INT32);;
	    64)
		AC_DEFINE(HAS_INT64)
		TYPE_INT64=${typName}
		AC_SUBST(TYPE_INT64);;
	esac
    fi
done
rm -rf ./szchk ./szchk.*
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# check size of integer enough to hold size of void *

AC_MSG_CHECKING([integer type enough to hold void pointer])
${CC} -I. ${CPPFLAGS} ${CFLAGS} ${hasLLFlags} ./utils/chkvoidp.c -o chkvoidp
voidPint="unknown"
if test -x ./chkvoidp; then
    voidPint=`./chkvoidp`
else
    AC_MSG_RESULT([can't create check program. exit.])
    exit 1
fi
if test "x${voidPint}" = "xunknown"; then
    AC_MSG_RESULT([can't determine????])
    exit 1
else
    AC_MSG_RESULT([$voidPint])
fi
INT_ENUFF_FOR_VOIDP=$voidPint
AC_SUBST(INT_ENUFF_FOR_VOIDP)
rm -f ./chkvoidp.* ./chkvoidp
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# check 64 bit virtual address

AC_MSG_CHECKING([64 bit virtual address])
${CC} -I. ${CPPFLAGS} ${CFLAGS} ./utils/addr64.c -o ./addr64
addrIs64="unknown"
if test -x ./addr64; then
    addrIs64=`./addr64`
else
    AC_MSG_RESULT([can't create check program. exit.])
    exit 1
fi
if test "x$addrIs64" = "xunknown"; then
    AC_MSG_RESULT([can't determine????])
    exit 1
else
    if test "x$addrIs64" = "x8"; then
	AC_MSG_RESULT([yes])
	AC_DEFINE(ADDR_IS_64)
    else
	AC_MSG_RESULT([no])
    fi
fi
rm -f ./addr64.* ./addr64
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# has printf "%qd"

rm -f ./hasqd.* ./hasqd
AC_MSG_CHECKING([has quad-precision print format])
${CC} -I. ${CPPFLAGS} ${CFLAGS} ./utils/hasqd.c -o ./hasqd > /dev/null 2>&1
hasQD=no
if test -x ./hasqd; then
    ret=`./hasqd`
    if test "x$ret" = "xqd" -o "x$ret" = "x%qd"; then
	hasQD=no
    else
	hasQD=yes
    fi
fi
AC_MSG_RESULT([${hasQD}])
if test "x${hasQD}" = "xyes"; then
    AC_DEFINE(HAS_QUAD_PRINT)
fi
rm -f ./hasqd.* ./hasqd
# --------------------------------------------------------------------


# --------------------------------------------------------------------
# Various command check

# Check ranlib
AC_PROG_RANLIB
if test "x${isGcc}" = "xno"; then
    case $OS in
	solaris*|irix*)
	    RANLIB=':';;
    esac
fi

if test "${OS}" = "cygwin32"; then
    INSTALLCOM='install -c'
    MKDIRCOM='mkdir -p'
else
    INSTALLCOM='$(TOPDIR)/utils/install-sh -c'
    MKDIRCOM='$(TOPDIR)/utils/mkdirhier'
fi

AR="ar cr"

YACCCOM=""
AC_PATH_PROG(YACCCOM, bison, yacc, ${PATH}:/usr/local/bin:/opt/bin:/opt/local/bin:/opt/gnu/bin:/usr/bin:/bin)
if test "`basename $YACCCOM`" = "bison"; then
    YACCCOM="$YACCCOM -y"
fi

AC_SUBST(AR)

AC_SUBST(INSTALLCOM)
AC_SUBST(MKDIRCOM)
AC_SUBST(YACCCOM)
# --------------------------------------------------------------------
XMPCC=""
XMPLIBDIR=""
XMPINCDIR=""
AC_PATH_PROG(XMPCC, xmpcc,  ${PATH}:/usr/local/bin:/opt/bin:/opt/local/bin:/opt/gnu/bin:/usr/bin:/bin)
if test $XMPCC == ""; then 
  echo "WARNING: cannot find xmpcc"
else
  xmpcc_bindir=`dirname $XMPCC`
  xmpcc_prefix=`dirname $xmpcc_bindir`
  XMPLIBDIR=$xmpcc_prefix/lib
  XMPINCDIR=$xmpcc_prefix/include
  xmplib=$XMPLIBDIR/libxmp.a
fi

if test -f "$xmplib"
then
 echo $xmplib
else
  echo "WARNING: cannot find xmp library"
fi
if test -f "$XMPINCDIR/xmp.h"
then
 echo $XMPINCDIR/xmp.h
else
  echo "WARNING: cannot find xmp header"
fi

AC_SUBST(XMPLIBDIR)
AC_SUBST(XMPCC)
AC_SUBST(XMPINCDIR)

# --------------------------------------------------------------------
# Finally, sed.

AC_OUTPUT(
Makefile
include/Makefile
src/libomrpc/Makefile
src/libomrpc/omrpc_defs.h
src/omrpc-agent/Makefile
src/omrpc-gen/Makefile
src/omrpc-gen/omrpc-cc
src/omrpc-gen/omrpc-fc
src/omrpc-gen-xmp/Makefile
src/omrpc-gen-xmp/omrpc-cc-xmp
src/omrpc-register/Makefile
src/omrpc-register-yml/Makefile
src/omrpc-register-yml/omrpc_reg_defs.h
test/call-test/Makefile
test/exec-test/Makefile
test/stub-test/Makefile
)

