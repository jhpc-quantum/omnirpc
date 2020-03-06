# Generated automatically from Makefile.in by configure.
# $Id: Makefile.in,v 1.3 2006-01-25 16:06:18 ynaka Exp $
# $Release: omnirpc-2.0.1 $
# $Copyright:
#  OmniRPC Version 1.0
#  Copyright (C) 2002-2004 HPCS Laboratory, University of Tsukuba.
#  
#  This software is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License version
#  2.1 published by the Free Software Foundation.
#  
#  Please check the Copyright and License information in the files named
#  COPYRIGHT and LICENSE under the top  directory of the OmniRPC Grid PRC 
#  System release kit.
#  
#  
#  $

TOPDIR		=	../..
prefix		=	$(DESTDIR)/home/tsuji/local/MUST3
exec_prefix	=	${prefix}

bindir		=	$(exec_prefix)/bin
libdir		=	$(prefix)/lib

INSTALLDIR	=	$(bindir)
INSTALL		= 	$(TOPDIR)/utils/install-sh -c
MKDIR		=	$(TOPDIR)/utils/mkdirhier

GCCINC		=	-I`case "$(CC)" in *gcc) echo \`$(CC) -v 2>&1 | grep specs | sed 's/specs$$/include/' | awk '{ print $$NF }'\`;; esac`


CC_OPT_SWITCH	=	 -Wall -fPIC 
CPPFLAGS	=	-I. -I$(TOPDIR)/include -I../libomrpc -DHAVE_ALLOCA_H=1 -DHAVE_ALLOCA=1 -DHAVE_STDARG_H=1 -DHAVE_LOCALE_H=1 -DHAVE_SETLOCALE=1 -DNO_IEEEFP_H=1 -DHAVE_STRDUP=1 -DHAVE_SYS_TIME_H=1 -DTIME_WITH_SYS_TIME=1 -DSTDC_HEADERS=1 -DHAVE_INET_ATON=1 -DHAS_LONGLONG=1 -DHAS_LONGDOUBLE=1 -DSIZEOF_UNSIGNED_SHORT=2 -DSIZEOF_UNSIGNED_INT=4 -DSIZEOF_UNSIGNED_LONG=8 -DSIZEOF_UNSIGNED_LONG_LONG=8 -DSIZEOF_FLOAT=4 -DSIZEOF_DOUBLE=8 -DSIZEOF_LONG_DOUBLE=16 -DSIZEOF_VOID_P=8 -DCHAR_ALIGN=1 -DSHORT_ALIGN=2 -DINT_ALIGN=4 -DLONG_ALIGN=8 -DLONGLONG_ALIGN=8 -DFLOAT_ALIGN=4 -DDOUBLE_ALIGN=8 -DLONGDOUBLE_ALIGN=16 -DHAS_INT16=1 -DHAS_INT32=1 -DHAS_INT64=1 -DADDR_IS_64=1 -DHAS_QUAD_PRINT=1   -DOMNI_CPU_X86_64 -DOMNI_OS_LINUX -DNDEBUG  -D__USE_FIXED_PROTOTYPES__ -D_REENTRANT
CFLAGS		=	$(CC_OPT_SWITCH)

CC		=	mpicc 
AR		=	ar cr
RANLIB		=	ranlib

YACC		=	yacc

DEPEND_SWITCH	=	$(CPPFLAGS)

SRCS = omrpc_agent_main.c omrpc_agent_job.c omrpc_agent_mxio.c \
	omrpc_agent_globus.c

OBJS = omrpc_agent_main.o omrpc_agent_job.o omrpc_agent_mxio.o \
	omrpc_agent_globus.o

LIBS = -L../libomrpc -lomrpc_stub -lomrpc_io  -lpthread \
	

TARGET = omrpc-agent

all: $(TARGET)

# 
#  OmniRPC agent program
#

$(TARGET): $(OBJS)
	$(CC) $(CC_OPT_SWITCH) -o $(TARGET) $(OBJS) $(LIBS)

install: all
	@if [ ! -d $(INSTALLDIR) ] ; then \
		(echo "Making directory $(INSTALLDIR)" ; \
		$(MKDIR) $(INSTALLDIR) ; \
		chmod 775 $(INSTALLDIR)); \
	else \
		true ; \
	fi
	for i in $(TARGET) ; do \
		$(INSTALL) -m 755 $$i $(INSTALLDIR); \
	done

clean:
	rm -f *.o *~ core $(TARGET)

distclean:	clean
	rm -f config.* Makefile .depend*

depend: $(SRCS)
	touch .depend.tmp
	makedepend -f ./.depend.tmp -a -- $(GCCINC) $(DEPEND_SWITCH) -- $(SRCS)
	sed 's:^$(PLATFORMDIR)/:./:' ./.depend.tmp > ./.depend
	cat Makefile ./.depend > ./Makefile.tmp
	mv Makefile.tmp Makefile
	rm -f ./.depend ./.depend.tmp .depend.tmp.bak

Makefile::
	(cd $(TOPDIR); sh ./config.status)

