TOPDIR          = /home/m-hirano/src/omnirpc
MKRULESDIR      = /home/m-hirano/src/omnirpc/mk

include $(MKRULESDIR)/vars.mk

SRCS	=	qcs_qulacs.cpp qcs_api.cpp
 
LDFLAGS	+=	 $(QULACS_LIBS)
#LDFLAGS	+=	 $(OMRPC_STUBLIB)

TARGET_LIB	=	libqcs.la
INSTALL_LIB_TARGETS	=	$(TARGET_LIB)

all::	$(INSTALL_LIB_TARGETS) apitest

apitest:	main.o
	$(LTLINK_CXX) -o $@ $< -R $(PWD)/.libs -L.libs -lqcs $(OMRPC_CLIENTLIB)

include $(MKRULESDIR)/rules.mk

clean::
	$(RM) $(INSTALL_LIB) apitest *.o *~

