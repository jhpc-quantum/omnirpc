TOPDIR          = /home/u12066/local/local.x86_64/src/omnirpc
MKRULESDIR      = /home/u12066/local/local.x86_64/src/omnirpc/mk

include $(MKRULESDIR)/vars.mk

SRCS	=	qcs_qulacs.cpp qcs_api.cpp
 
LDFLAGS	+=	 $(QULACS_LIBS)

TARGET_LIB	=	libqcs.la
INSTALL_LIB_TARGETS	=	$(TARGET_LIB)

all::	$(INSTALL_LIB_TARGETS) apitest

apitest:	main.o
	$(LTLINK_CXX) -o $@ $< -R $(PWD)/.libs -L.libs -lqcs $(OMRPC_CLIENTLIB)

include $(MKRULESDIR)/rules.mk

clean::
	$(RM) $(INSTALL_LIB) apitest *.o *~

