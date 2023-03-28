A brief description of qc-rpc usage

0. Prerequisite

  Innstall Qulacs
  	https://github.com/qulacs/qulacs.git
	   
1. Build

  At the source tree top directory, run the configure:
	$ ./configure --prefix=/whare/to/install \
	  --enable-qcrpc \
	  --with-qulacs=/whare/the/qulacs/insralled

2. Sample code

the following is a sample code covering whole rthe qc-rpc APIs,
existig in QC_API directory main.c:

----------------------------------------------------------------
     1	#include "omni_platform.h"
     2	#include "qcs_api.hpp"
     3	
     4	int main(int argc, char *argv[])
     5	{
     6	  int qubits = 5;
     7	  bool do_save = false;
     8	  bool do_load = false;
     9	  bool do_rpc = false;
    10	  bool do_job_submit = false;
    11	  
    12	  if (argc == 2 && strcmp(argv[1], "-s") == 0) {
    13	    do_save = true;
    14	  } else if (argc == 2 && strcmp(argv[1], "-l") == 0) {
    15	    do_load = true;
    16	  } else if (argc == 2 && strcmp(argv[1], "-re") == 0) {
    17	    do_rpc = true;
    18	  } else if (argc == 2 && strcmp(argv[1], "-rs") == 0) {
    19	    do_job_submit = true;
    20	    do_rpc = true;
    21	  }
    22	
    23	  if (do_rpc == true) {
    24	    QC_InitRemote(&argc, &argv);
    25	  }
    26	  
    27	  QC_Init(&argc, &argv, qubits, 0); // 0 = qulacs
    28	
    29	  if (do_load == false) {
    30	
    31	    HGate(0);
    32	    U1Gate(0.1, 0);
    33	    U2Gate(0.1, 0.2, 1);
    34	    U3Gate(0.1, 0.2, 0.3, 2);
    35	
    36	    if (do_save == true) {
    37	      QC_SaveContext("test.dump");
    38	    } else if (do_rpc == true) {
    39	      QC_MeasureRemote(do_job_submit);
    40	    }
    41	  } else {
    42	    QC_LoadContext("test.dump");
    43	  }
    44	
    45	  if (do_save == false && do_rpc == false) {
    46	    QC_Measure();
    47	  }
    48	
    49	  QC_Finalize();
    50	}
    51	
----------------------------------------------------------------

  Line 24, QC_InitRemote() is called to initilize OmniRPC layer. Call
this if you need qcsim run on an RPC server (w/ slurm.) The call must
be the first function call of the application.

  Line 27, QC_Init() is called to initialize QC_API layer. All the
application must call this function before using QC_APIs other than
QC_InitRemote().

  Line 37 and 42, QC_{Save|Load}Context() is used to save/restore the
qcsim application circuit configuration information to/from a
file. These are mainly used on an RPC server/slurm side.
  
  Line 39, QC_MeasureRemote() starts the simulation (measurement) with
given circuit configuratioon. The do_job_submit argiment controls how
the simuration runs via, '1' for via slurm, '0' for OmniRPC REX direct
execution.

  Line 46, contrary to QC_MeasureRemote(), QC_Measure() starts the
simuration on the local machine.

  Finally line 49, QC_Finalize() finalizes the QC_API layer.

3. Compiling applications

  All the needed headers are installed in ${prefix}/include, and all
the needed libraries are installed in ${prefix}/lib directory. Link
libqcs.so, libomrpc_client.so, and libomrpc_io.so.

e.g.
	LIBDIR	=	$(PREFIX)/lib
	CPPFLAGS	+=	-I$(PREFIX)/include
	LDFLAGS += -Wl,-rpath -Wl,$(LIBDIR) -L$(LIBDIR} \
		-lqcs -lomprpc_clinet -lomprpc_io



