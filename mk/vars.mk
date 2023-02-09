TARGET_ARCH	= 

SHELL		= /usr/local/bin/bash

srcdir		= .
TOPDIR		= /home/m-hirano/src/omnirpc
top_builddir	?= $(TOPDIR)
MKRULESDIR	= /home/m-hirano/src/omnirpc/mk

prefix		= /usr/local
sysconfdir	= ${prefix}/etc
rundir		= ${localstatedir}/run
vardir		= ${prefix}/var

ifneq ($(DESTDIR),)
prefix		:= $(DESTDIR)/$(prefix)
sysconfdir	:= $(DESTDIR)/$(sysconfdir)
endif

PREFIX		=	$(prefix)
SYSCONFDIR	=	$(sysconfdir)
RUNDIR		=	$(rundir)
VARDIR		=	$(vardir)

DEST_EXEDIR	= $(prefix)/bin
DEST_SBIN_EXEDIR	= $(prefix)/sbin
DEST_LIBDIR	= $(prefix)/lib
DEST_HDRDIR_TOP	= $(prefix)/include

LIBTOOL_DEPS	= ././mk//ltmain.sh

MAKE		= LC_ALL=C make
LN_S		= ln -nfs
READLINK		= readlink
BASENAME		= basename
RM		= rm -f
MKDIR		= mkdir -p
LIBTOOL		= LC_ALL=C $(SHELL) $(top_builddir)/libtool --quiet

CC		= gcc
CXX		= g++
LINK_CC		= $(CC)
LINK_CXX	= $(CXX)
YACC		= @YACC@
LEX		= @LEX@
AWK		= gawk

PKGCONF	= pkg-config

INSTALL		= /usr/bin/install -c
INSTALL_DATA	= ${INSTALL} -m 644
INSTALL_SCRIPT	= ${INSTALL}
INSTALL_PROGRAM	= ${INSTALL}

BUILD_SRCDIR	= $(TOPDIR)/src
BUILD_INCDIR	= $(BUILD_SRCDIR)/include
BUILD_LIBDIR	= $(BUILD_SRCDIR)/lib
BUILD_DPDKDIR = $(BUILD_SRCDIR)/dpdk
BUILD_DPDK_RXDIR = $(BUILD_SRCDIR)/dpdk-rx
BUILD_DPDK_TXDIR = $(BUILD_SRCDIR)/dpdk-tx
BUILD_RDMADIR = $(BUILD_SRCDIR)/rdma
BUILD_RDMA_RXDIR = $(BUILD_SRCDIR)/rdma-rx
BUILD_RDMA_TXDIR = $(BUILD_SRCDIR)/rdma-tx
BUILD_THD_POOLDIR = $(BUILD_SRCDIR)/thd-pool

OSDEF	=	OMNIRPC_OS_LINUX
CPUDEF	=	OMNIRPC_CPU_X86_64

SITECONF_MK	?=	$(shell ls $(MKRULESDIR)/siteconf.mk 2>/dev/null)
ifdef SITECONF_MK
__SITECONF__=.pre.
include $(SITECONF_MK)
endif

TOOLCHAIN_TARGET	= x86_64-linux-gnu

RDMA_LIBS		= @RDMA_LIBS@
LIBAIO_LIBS		= @LIBAIO_LIBS@

LIBFABRIC_DIR	= @LIBFABRIC_DIR@
ifneq ($(LIBFABRIC_DIR),)
LIBFABRIC_LIBS	= -L@LIBFABRIC_DIR@/lib -lfabric
LIBFABRIC_INCLUDE	= -I@LIBFABRIC_DIR@/include
endif

DPDK_DIR		= @DPDK_DIR@
ifneq ($(DPDK_DIR),)
DPDK_LIBS	= $(shell PKG_CONFIG_PATH=$(DPDK_DIR)/pkgconfig $(PKGCONF) --libs libdpdk)
DPDK_INCLUDE	= $(shell PKG_CONFIG_PATH=$(DPDK_DIR)/pkgconfig $(PKGCONF) --cflags libdpdk)
endif

SPDK_DIR		= @SPDK_DIR@
ifneq ($(SPDK_DIR),)
SPDK_INCLUDE		= -I@SPDK_DIR@/include
SPDK_LIBS		= -L@SPDK_DIR@/lib -lspdk -lspdk_env_dpdk -lisal
endif

PCAP_LIBS		= -lpcap 

FABR_UTIL_LIB		= libfabr_util.la
DEP_FABR_UTIL_LIB	= $(BUILD_LIBDIR)/$(FABR_UTIL_LIB)
RUNPATH_FABR_UTIL_LIB	=	$(BUILD_LIBDIR)/.libs

FABR_DPDK_LIB		= libfabr_dpdk.la
DEP_FABR_DPDK_LIB	= $(BUILD_DPDKDIR)/$(FABR_DPDK_LIB)
RUNPATH_FABR_DPDK_LIB	=	$(BUILD_DPDKDIR)/.libs

FABR_DPDK_RX_LIB		= libfabr_dpdk_rx.la
DEP_FABR_DPDK_RX_LIB	= $(BUILD_DPDK_RXDIR)/$(FABR_DPDK_RX_LIB)
RUNPATH_FABR_DPDK_RX_LIB	=	$(BUILD_DPDK_RXDIR)/.libs

FABR_DPDK_TX_LIB		= libfabr_dpdk_tx.la
DEP_FABR_DPDK_TX_LIB	= $(BUILD_DPDK_TXDIR)/$(FABR_DPDK_TX_LIB)
RUNPATH_FABR_DPDK_TX_LIB	=	$(BUILD_DPDK_TXDIR)/.libs

FABR_RDMA_LIB		= libfabr_rdma.la
DEP_FABR_RDMA_LIB	= $(BUILD_RDMADIR)/$(FABR_RDMA_LIB)
RUNPATH_FABR_RDMA_LIB	=	$(BUILD_RDMADIR)/.libs

FABR_RDMA_RX_LIB		= libfabr_rdma_rx.la
DEP_FABR_RDMA_RX_LIB	= $(BUILD_RDMA_RXDIR)/$(FABR_RDMA_RX_LIB)
RUNPATH_FABR_RDMA_RX_LIB	=	$(BUILD_RDMA_RXDIR)/.libs

FABR_RDMA_TX_LIB		= libfabr_rdma_tx.la
DEP_FABR_RDMA_TX_LIB	= $(BUILD_RDMA_TXDIR)/$(FABR_RDMA_TX_LIB)
RUNPATH_FABR_RDMA_TX_LIB	=	$(BUILD_RDMA_TXDIR)/.libs

FABR_THD_POOL_LIB		= libfabr_thd_pool.la
DEP_FABR_THD_POOL_LIB	= $(BUILD_THD_POOLDIR)/$(FABR_THD_POOL_LIB)
RUNPATH_FABR_THD_POOL_LIB	=	$(BUILD_THD_POOLDIR)/.libs

SUBMODULES	=

IS_DEVELOPER	= 

ifdef TOOLCHAIN_TARGET
CPPFLAGS	+= -I/usr/include/x86_64-linux-gnu
endif
CPPFLAGS	+= -D_REENTRANT -D_GNU_SOURCE -D_POSIX_SOURCE
CPPFLAGS	+= -I$(BUILD_INCDIR)

WARN_BASE_CFLAGS	+= -W -Wall -Wextra \
	-Wshadow \
	-Wcast-align \
	-Wwrite-strings \
	-Wconversion \
	-Waddress \
	-Wmissing-format-attribute \
	-Wno-long-long \
	-Wno-variadic-macros
#	-Wlogical-op

WARN_CFLAGS	+= 	-std=gnu99 \
	$(WARN_BASE_CFLAGS) \
	-Wstrict-prototypes \
	-Wold-style-definition \
	-Wmissing-declarations \
	-Wmissing-prototypes \
	-Wnested-externs \
	-Wdeclaration-after-statement

WARN_CXXFLAGS	+=	-std=gnu++11 \
	$(WARN_BASE_CFLAGS) \
	-Wnon-virtual-dtor \
	-Wstrict-null-sentinel \
	-Woverloaded-virtual

DEBUG_CFLAGS	?= -g3 -fno-omit-frame-pointer
DEBUG_CXXFLAGS	?= -g3 -fno-omit-frame-pointer

OPT_CFLAGS	?= -O0 
OPT_CXXFLAGS	?= -O0

CODEGEN_CFLAGS		+= -fkeep-inline-functions
CODEGEN_CXXFLAGS	+= -fkeep-inline-functions

ifdef IS_DEVELOPER
DEVELOPER_CFLAGS	+= -fsanitize=address
DEVELOPER_CXXFLAGS	+= -fsanitize=address
CPPFLAGS	+=	-D__UNDER_SANITIZER__
endif

COMMON_CFLAGS	= $(WARN_CFLAGS) $(DEBUG_CFLAGS) $(OPT_CFLAGS) \
	$(CODEGEN_CFLAGS) $(LOCAL_CFLAGS) $(DEVELOPER_CFLAGS)

COMMON_CXXFLAGS	= -std=gnu++98 $(WARN_CXXFLAGS) $(DEBUG_CXXFLAGS) \
	$(OPT_CXXFLAGS) $(CODEGEN_CXXFLAGS) $(LOCAL_CXXFLAGS) $(DEVELOPER_CXXFLAGS)

CFLAGS		+=  -g -O2 $(COMMON_CFLAGS)
CXXFLAGS	+=  -g -O2 $(COMMON_CXXFLAGS)

LDFLAGS		+= -Wl,--as-needed \
		    -lrt -lpthread -ldl 

OBJS		?= $(filter-out $(SRCS),$(SRCS:.c=.lo)) \
			$(filter-out $(SRCS),$(SRCS:.cc=.lo)) \
			$(filter-out $(SRCS),$(SRCS:.cpp=.lo))

ifdef TARGET_LIB
SHLIB_VERSION	?= 0:0:0
LTLINK_VERSION	= -version-info $(SHLIB_VERSION)
endif
LTLINK_RPATH	= -rpath $(DEST_LIBDIR)
LTLINK_RUNPATH	=

ifneq ($(DPDK_DIR),)
LTLINK_RUNPATH  += -R $(DPDK_DIR)
endif

ifneq ($(SPDK_DIR),)
LTLINK_RUNPATH  += -R $(SPDK_DIR)/lib
endif

ifneq ($(LIBFABRIC_DIR),)
LTLINK_RUNPATH  += -R $(LIBFABRIC_DIR)/lib
endif

LTLINK_RUNPATH	+= \
	-R $(RUNPATH_FABR_UTIL_LIB) \
	-R $(RUNPATH_FABR_THD_POOL_LIB) \
	-R $(RUNPATH_FABR_DPDK_LIB)\
	-R $(RUNPATH_FABR_DPDK_RX_LIB) \
	-R $(RUNPATH_FABR_DPDK_TX_LIB) \
  -R $(RUNPATH_FABR_RDMA_LIB) \
	-R $(RUNPATH_FABR_RDMA_RX_LIB) \
  -R $(RUNPATH_FABR_RDMA_TX_LIB)

LTLINK_RUNPATH	+= -R $(DEST_LIBDIR)

STATICBUILD	=	@STATICBUILD@
ifndef STATICBUILD
LT_SHARED	=	-shared
endif

LTCOMPILE_CC	= $(LIBTOOL) --mode=compile --tag=CC $(CC) $(CFLAGS) \
	$(CPPFLAGS) $(LT_SHARED)
LTCOMPILE_CXX	= $(LIBTOOL) --mode=compile --tag=CXX $(CXX) $(CXXFLAGS) \
	$(CPPFLAGS) $(LT_SHARED)
LTLIB_CC	= $(LIBTOOL) --mode=link --tag=CC \
	$(LINK_CC) $(CFLAGS) $(LTLINK_RPATH) $(LTLINK_VERSION) $(LT_SHARED)
LTLIB_CXX	= $(LIBTOOL) --mode=link --tag=CXX \
	$(LINK_CXX) $(CXXFLAGS) $(LTLINK_RPATH) $(LTLINK_VERSION) $(LT_SHARED)
LTLINK_CC	= $(LIBTOOL) --mode=link --tag=CC \
	$(LINK_CC) $(CFLAGS) $(LTLINK_RUNPATH) $(LT_SHARED)
LTLINK_CXX	= $(LIBTOOL) --mode=link --tag=CXX \
	$(LINK_CXX) $(CXXFLAGS) $(LTLINK_RUNPATH) $(LT_SHARED)
LTINSTALL_EXE	= $(LIBTOOL) --mode=install $(INSTALL_PROGRAM)
LTINSTALL_LIB	= $(LIBTOOL) --mode=install $(INSTALL_DATA)
LTINSTALL_HEADER= $(LIBTOOL) --mode=install $(INSTALL_DATA)
LTCLEAN		= $(LIBTOOL) --mode=clean $(RM)

HAVE_RUBY		= 
RUBY			= ruby

HAVE_PROTOC		= @HAVE_PROTOC@
HAVE_UNITY		= @HAVE_UNITY@
TEST_RESULT_DIR		= $(TOPDIR)/test_result_dir
UNITY_DIR		= $(TOPDIR)/tools/unity
BUILDUNITY_DIR		= $(UNITY_DIR)/src
UNITY_CPPFLAGS		= -I$(BUILDUNITY_DIR)
UNITY_GEN_RUNNER	= $(UNITY_DIR)/auto/generate_test_runner.rb
UNITY_SUMMARY		= $(UNITY_DIR)/auto/unity_test_summary.rb
UNITY_REPORT		= $(UNITY_DIR)/auto/colour_reporter
UNITY_LIB		= libunity.la
DEP_UNITY_LIB		= $(BUILDUNITY_DIR)/$(UNITY_LIB)
RUNPATH_UNITY_LIB	= $(BUILDUNITY_DIR)/.libs

HAVE_GCOVR		= 

HAVE_PAPI		= @HAVE_PAPI@

ifdef SITECONF_MK
__SITECONF__=.post.
include $(SITECONF_MK)
endif