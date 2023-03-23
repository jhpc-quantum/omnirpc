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
# --------------------------------------------------------------------------

##
## Autoconf template.
##
AC_INIT

TOPDIR=${ac_pwd}
AC_SUBST(TOPDIR)

AC_CONFIG_AUX_DIR(./mk)
AC_CONFIG_MACRO_DIR(./mk)
m4_include([mk/libtool.m4])
m4_include([mk/ltoptions.m4])
m4_include([mk/ltsugar.m4])
m4_include([mk/ltversion.m4])
m4_include([mk/lt~obsolete.m4])

AC_CONFIG_HEADERS([include/omni_config.h])

if test "${prefix}" = "NONE"; then
    prefix=/usr/local
fi
if test "${exec_prefix}" = "NONE"; then
    exec_prefix='${prefix}'
fi

PREFIX=${prefix}
AC_SUBST(PREFIX)

case `uname` in
Linux)
    SHELL=`which bash` > /dev/null 2>&1
    if test $? -ne 0 -o -z "${SHELL}"; then
        AC_MSG_ERROR([To run libtool you need the GNU bash, not the old-school /bin/sh.])
        exit 1
    fi
;;
*)
    AC_MSG_RESULT([SHELL is "$SHELL". bash is recommended.])
;;
esac

export SHELL
AC_SUBST(SHELL)

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
#AC_PROG_LEX
#if test "x$LEX" != xflex -a "x$LEX" != xlex; then
#    AC_MSG_ERROR([flex/lex must be installed.])
#fi
AC_PROG_AWK

RM="rm -f"
AC_SUBST(RM)

MKDIR="mkdir -p"
AC_SUBST(MKDIR)

LN_S="ln -nfs"
AC_SUBST(LN_S)

READLINK="readlink"
AC_SUBST(READLINK)

BASENAME="basename"
AC_SUBST(BASENAME)

PKGCONF="pkg-config"
AC_SUBST(PKGCONF)

# Assume 32 bit for cross compilation.
AC_CHECK_SIZEOF(short int, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long int, 4)
AC_CHECK_SIZEOF(long long, 8)
AC_CHECK_SIZEOF(int64_t, 8)
AC_CHECK_SIZEOF(size_t, 4)
AC_CHECK_SIZEOF(void *, 4)
AC_CHECK_SIZEOF(pthread_t, 4)

# sysdeps.

eval `echo ${target} | awk -F- '{ printf "tagCPU=%s; tagOS=%s;\n", $1, $3 }'`

changequote(<<, >>)dnl
CPU=`echo ${tagCPU} | tr '[A-Z]' '[a-z]'`
OS=`echo ${tagOS} | tr '[A-Z]' '[a-z]'`
changequote([, ])dnl
unset tagOS tagCPU

case $CPU in
    i*86)
        CPU=i386;;
esac

OS_CPPFLAGS='-D_REENTRANT'
OS_LDFLAGS=''
OS_LIBS=''
OS_CFLAGS=''
if test -z "${MAKE}"; then
	MAKE=make
fi

ECHO_N='echo -n'
STAT='stat -c %a'

case $OS in
    sunos*)
        OS=sunos;;
    solaris*)
        OS=solaris;;
    freebsd*)
	MAKE=gmake
	OS_CPPFLAGS="${OS_CPPFLAGS} -I/usr/local/include"
	OS_LDFLAGS="${OS_LDFLAGS} -L/usr/local/lib"
	CPPFLAGS="${CPPFLAGS} ${OS_CPPFLAGS}"
	LDFLAGS="${LDFLAGS} ${OS_LDFLAGS}"
	OS_LIBS="-lpthread -lexecinfo"
	OS=freebsd;;
    linux*)
	OS_CPPFLAGS="${OS_CPPFLAGS} -D_GNU_SOURCE -D_POSIX_SOURCE"
	OS_LIBS="-lrt -lpthread -ldl"
	OS=linux;;
    netbsd*)
	MAKE=gmake
	OS_CPPFLAGS="${OS_CPPFLAGS} -I/usr/pkg/include"
	OS_LDFLAGS="${OS_LDFLAGS} -L/usr/pkg/lib"
	CPPFLAGS="${CPPFLAGS} ${OS_CPPFLAGS}"
	LDFLAGS="${LDFLAGS} ${OS_LDFLAGS}"
	OS_LIBS="-lpthread -lexecinfo"
	OS=netbsd;;
    darwin*)
	ECHO_N='/bin/echo -n'
	STAT='stat -f %p'
	OS_CPPFLAGS="${OS_CPPFLAGS} -D_GNU_SOURCE -D_DARWIN_C_SOURCE"
	OS_LIBS="-lpthread"
	OS=darwin;;
esac

# checking CC
TOOLCHAIN_TARGET=''
if test -z "${CC}"; then
    CC=gcc
fi
AC_MSG_CHECKING([toolchain target arch])
_v=`${CC} -v 2>&1 | egrep '^Target:' | awk '{ print $NF }'`
if test $? -eq 0 -a ! -z "${_v}"; then
    case ${CC} in
    	*gcc)
	    TOOLCHAIN_TARGET=${_v};;
    esac
    if test -z "${TOOLCHAIN_TARGET}"; then
        AC_MSG_RESULT([unknown, yet.])
    else
	AC_MSG_RESULT([${TOOLCHAIN_TARGET}])
    fi
fi
AC_SUBST(TOOLCHAIN_TARGET)

AC_SUBST(MAKE)
AC_SUBST(OS_CPPFLAGS)
AC_SUBST(OS_CFLAGS)
AC_SUBST(OS_LDFLAGS)
AC_SUBST(OS_LIBS)
AC_SUBST(ECHO_N)
AC_SUBST(STAT)

changequote(<<, >>)dnl
CPUDEF=OMNIRPC_CPU_`echo $CPU | tr '[a-z]' '[A-Z]'`
OSDEF=OMNIRPC_OS_`echo $OS | tr '[a-z]' '[A-Z]'`
changequote([, ])dnl
AC_SUBST(CPUDEF)
AC_SUBST(OSDEF)

MKRULESDIR=${TOPDIR}/mk
AC_SUBST(MKRULESDIR)

AC_SUBST(CPPFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(TARGET_ARCH)

AC_SUBST(IS_DEVELOPER)
AC_SUBST(HAVE_RUBY)
AC_SUBST(HAVE_NKF)
AC_SUBST(HAVE_GCOVR)

AC_MSG_CHECKING([/proc/self/exe])
if test -h /proc/self/exe; then
  AC_MSG_RESULT([exists.])
  AC_DEFINE(HAVE_PROCFS_SELF_EXE)
else
  AC_MSG_RESULT([does noe exists.])
fi

# headers/macros

#oLIBS=${LIBS}
#LIBS=''
#AC_CHECK_HEADER(pcap/pcap.h,, [AC_MSG_ERROR([pcap/pcap.h is required!])])
#AC_CHECK_LIB(pcap, pcap_create,, [AC_MSG_ERROR([libpcap is required!])])
#PCAP_LIBS=${LIBS}
#AC_SUBST(PCAP_LIBS)
#LIBS=${oLIBS}

AC_C_BIGENDIAN

AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP


AC_CHECK_HEADER(inttypes.h, [AC_DEFINE(HAVE_INTTYPES_H)], [])
AC_CHECK_HEADER(stdint.h, [AC_DEFINE(HAVE_STDINT_H)])
AC_CHECK_HEADER(limits.h, [AC_DEFINE(HAVE_LIMITS_H)])
AC_CHECK_HEADER(sys/param.h, [AC_DEFINE(HAVE_SYS_PARAM_H)])
AC_CHECK_HEADER(sys/types.h, [AC_DEFINE(HAVE_SYS_TYPES_H)])
AC_CHECK_HEADER(ctype.h, [AC_DEFINE(HAVE_CTYPE_H)])
AC_CHECK_HEADER(stdio.h, [AC_DEFINE(HAVE_STDIO_H)])
AC_CHECK_HEADER(stdlib.h, [AC_DEFINE(HAVE_STDLIB_H)])
AC_CHECK_HEADER(unistd.h, [AC_DEFINE(HAVE_UNISTD_H)])
AC_CHECK_HEADER(memory.h, [AC_DEFINE(HAVE_MEMORY_H)])
AC_CHECK_HEADER(string.h, [AC_DEFINE(HAVE_STRING_H)])
AC_CHECK_HEADER(malloc.h, [AC_DEFINE(HAVE_MALLOC_H)])
AC_CHECK_HEADER(stdarg.h, [AC_DEFINE(HAVE_STDARG_H)])
AC_CHECK_HEADER(dirent.h, [AC_DEFINE(HAVE_DIRENT_H)])
AC_CHECK_HEADER(time.h, [AC_DEFINE(HAVE_TIME_H)])
AC_CHECK_HEADER(sys/time.h, [AC_DEFINE(HAVE_SYS_TIME_H)])
AC_CHECK_HEADER(termios.h, [AC_DEFINE(HAVE_TERMIOS_H)])
AC_CHECK_HEADER(sys/stat.h, [AC_DEFINE(HAVE_SYS_STAT_H)])
AC_CHECK_HEADER(fcntl.h, [AC_DEFINE(HAVE_FCNTL_H)])
AC_CHECK_HEADER(errno.h, [AC_DEFINE(HAVE_ERRNO_H)])
AC_CHECK_HEADER(signal.h, [AC_DEFINE(HAVE_SIGNAL_H)])
AC_CHECK_HEADER(netdb.h, [AC_DEFINE(HAVE_NETDB_H)])
AC_CHECK_HEADER(netinet/tcp.h, [AC_DEFINE(HAVE_NETINET_TCP_H)])
AC_CHECK_HEADER(sys/socket.h, [AC_DEFINE(HAVE_SYS_SOCKET_H)])
AC_CHECK_HEADER(arpa/inet.h, [AC_DEFINE(HAVE_ARPA_INET_H)])
AC_CHECK_HEADER(sys/ioctl.h, [AC_DEFINE(HAVE_SYS_IOCTL_H)])
AC_CHECK_HEADER(sys/wait.h, [AC_DEFINE(HAVE_SYS_WAIT_H)])
AC_CHECK_HEADER(pthread.h, [AC_DEFINE(HAVE_PTHREAD_H)])
AC_CHECK_HEADER(stdbool.h, [AC_DEFINE(HAVE_STDBOOL_H)])
AC_CHECK_HEADER(sys/resource.h, [AC_DEFINE(HAVE_SYS_RESOURCE_H)])
AC_CHECK_HEADER(regex.h, [AC_DEFINE(HAVE_REGEX_H)])
AC_CHECK_HEADER(syslog.h, [AC_DEFINE(HAVE_SYSLOG_H)])
AC_CHECK_HEADER(getopt.h, [AC_DEFINE(HAVE_GETOPT_H)])
AC_CHECK_HEADER(bsd/sys/queue.h, [AC_DEFINE(HAVE_BSD_SYS_QUEUE_H)])
AC_CHECK_HEADER(sys/queue.h, [AC_DEFINE(HAVE_SYS_QUEUE_H)])
AC_CHECK_HEADER(mcheck.h, [AC_DEFINE(HAVE_MCHECK_H)])
AC_CHECK_HEADER(assert.h, [AC_DEFINE(HAVE_ASSERT_H)])
AC_CHECK_HEADER(poll.h, [AC_DEFINE(HAVE_POLL_H)])
AC_CHECK_HEADER(sys/mman.h, [AC_DEFINE(HAVE_SYS_MMAN_H)])
AC_CHECK_HEADER(sys/prctl.h, [AC_DEFINE(HAVE_SYS_PRCTL_H)])

oLIBS=${LIBS}
LIBS="${LIBS} -lpthread"
AC_CHECK_FUNCS([strlcpy strlcat, pthread_mutex_timedlock])
LIBS=${oLIBS}
AC_CHECK_FUNC(strtold, [AC_DEFINE(HAVE_STRTOLD)])

### MPI
USE_MPI=''
mpi_flavor=''
AC_ARG_WITH([mpi],
	[AS_HELP_STRING([--with-mpi=flavor], [specify MPI type to use])],
	[mpi_flavor="$withval"], [mpi_flavor=''])
if test ! -z "${mpi_flavor}"; then
    AC_MSG_CHECKING([MPI])
    case ${mpi_flavor} in
        yes|openmpi) mpi_flavor=mpi;;
    esac
    mk/genflags.sh ${mpi_flavor} | sed 's:^PKG_:MPI_:' > mk/mpi.mk
    if test $? -eq 0; then
	USE_MPI=yes
	AC_DEFINE(USE_MPI)
	AC_MSG_RESULT([${mpi_flavor}.])
    else
        AC_MSG_ERROR([failed to find ${mpi_flavor}.])
    fi
fi
AC_SUBST(USE_MPI)

### qc-rpc
USE_QC_RPC=''
do_qc_rpc=no
AC_ARG_ENABLE([qcrpc],
	[AS_HELP_STRING([--enable-qcrpc], [Support qc-rpc feature [default: no]])],
	[do_qc_rpc="$enableval"], [do_qc_rpc=no])
AC_MSG_CHECKING([qc-rpc])
if test "x${do_qc_rpc}" != "xno"; then
    USE_QC_RPC=yes
    AC_DEFINE(USE_QC_RPC)
fi
AC_MSG_RESULT([${do_qc_rpc}.])
AC_SUBST(USE_QC_RPC)    

QULACS_SRCDIR=''
qulacs_dir=''
AC_ARG_WITH([qulacs],
	[AS_HELP_STRING([--with-qulacs], [Specify a top source directory of Qulacs.])],
	[qulacs_dir="$withval"], [qulacs_dir=''])
AC_MSG_CHECKING(qulacs)
if test "x${do_qc_rpc}" != "xno" -a -d ${qulacs_dir}/include -a -d ${qulacs_dir}/lib; then
    QULACS_SRCDIR=${qulacs_dir}
fi
AC_MSG_RESULT([${QULACS_SRCDIR}])
AC_SUBST(QULACS_SRCDIR)
if test "x${do_qc_rpc}" != "xno" -a -z "${QULACS_SRCDIR}"; then
    AC_MSG_ERROR([Qulacs source directory must be specified when --wnable-qcrpc.])
    exit 1
fi

### Globus (deprecated)
USE_GLOBUS=''
GLOBUS_LIBS=''
AC_SUBST(USE_GLOBUS)
AC_SUBST(GLOBUS_LIBS)

# emit.
AC_CONFIG_FILES(
	mk/vars.mk
	Makefile
	include/Makefile
	src/libomrpc/Makefile
	src/libomrpc/omrpc_defs.h
	src/omrpc-agent/Makefile
	src/omrpc-gen/Makefile
	src/omrpc-gen/omrpc-cc
	src/omrpc-gen/omrpc-fc
	src/omrpc-gen-mpi/Makefile
	src/omrpc-gen-mpi/omrpc-cc-mpi
	src/omrpc-gen-xmp/Makefile
	src/omrpc-gen-xmp/omrpc-cc-xmp
	src/omrpc-register/Makefile
	src/omrpc-register-yml/Makefile
	src/omrpc-register-yml/omrpc_reg_defs.h
	qcrpc/Makefile
	qcrpc/QC_API/Makefile
	qcrpc/qcrex/Makefile
	qcrpc/qcmeasure/Makefile
	test/call-test/Makefile
	test/exec-test/Makefile
	test/comm-test/Makefile
	test/stub-test/Makefile
	example/cpi/Makefile
	example/ep/Makefile
	example/knap/Makefile
)

# emit and set executable attribute.
AC_DEFUN([CONFIG_EXECS],
[
m4_foreach_w(
    [FILE_NAME],
    [$1],
    [AC_CONFIG_FILES(FILE_NAME, chmod +x FILE_NAME)])
])

CONFIG_EXECS(
	mk/initdepend
)

AC_CONFIG_COMMANDS_POST([rm -rf ./config.log ./config.status.* autom4te.cache])

AC_OUTPUT

sh mk/initdepend && ${MAKE} prerequisite 
if test $? -ne 0; then
    AC_MSG_ERROR([Prerequisite failure.])
fi

echo ""
echo "CC:	'${CC}'"
echo "CFLAGS:	'${OS_CFLAGS}'"
echo "CPPFLAGS:	'${CPPFLAGS} ${OS_CPPFLAGS}'"
echo "LDFLAGS:	'${LDFLAGS} ${OS_LDFLAGS} ${OS_LIBS}'"
echo ""
