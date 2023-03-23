TOPDIR          = /home/m-hirano/src/omnirpc
MKRULESDIR      = /home/m-hirano/src/omnirpc/mk

include $(MKRULESDIR)/vars.mk

SRCS	=	qcs_qulacs.cpp qcs_api.cpp
 
DEP_LIBS	=	-R $(QULACS_CSIM_LIBDIR) -L$(QULACS_CSIM_LIBDIR) -lcsim_shared -R$(QULACS_CPPSIM_LIBDIR) -L$(QULACS_CPPSIM_LIBDIR) -lcppsim_shared

TARGET_LIB	=	libqcs.la
INSTALL_LIB_TARGETS	=	$(TARGET_LIB)

all::	$(INSTALL_LIB_TARGETS) apitest

apitest:	main.o
	$(LTLINK_CXX) -o $@ $< -R $(PWD)/.libs -L.libs -lqcs

include $(MKRULESDIR)/rules.mk

clean::
	$(RM) $(INSTALL_LIB) apitest *.o *~

