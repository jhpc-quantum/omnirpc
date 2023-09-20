A brief description of qc-rpc usage on the Super Computer Fugaku

0. Prerequisite

a)  Fugaku specific rule

	The software installation and execution should be destinated
	to under /data/<gid>/<uid>/ directory. Create directories
	under this directory for convenience.

b)  Install QURI Parts riqu for RQC/QURI support on Fugaku

	QURI Parts riqu is required to execute jobs on RQC/QURI
	environment. It requires using spack and pip3, users must set
	PYTHONUSERENV environment variable before the
	installation. You can do this following steps on a Fugaku
	login node:

	$ export PYTHONUSERBASE=/data/<gid>/<uid>/somewhere
	$ . /vol0004/apps/oss/spack/share/spack/setup-env.sh
	$ spack load python@3.10.8/yt6afcn
	$ spack load py-pip@23.0/wyuv6uh

c)  Build Qulacs:

		https://github.com/qulacs/qulacs.git

	This is needed in any environment including Fugaku. For most
	environments, the easiest way to build this system is to issue
	./script/build_gcc.sh under the cloned git directory.

1. Build

  At the source tree top directory, run the configure:

	$ ./configure --prefix=/whare/to/install \
	  --enable-qcrpc \
	  --enable-homeshare \
	  --with-quri=/data/<gid>/<uid>/somewhere \
	  --with-qulacs=/whare/the/qulacs/insralled

  The --enable-homeshare option is for separating ~/.omrpc_regitry by
host architectures by augmenting the registry directory name by `uname
-i` output, like ~/.omrpc_registry.aarch64. It is helpful on Fugaku or
any other environments where the user home directories are shared
between different architecture machines.

2. Sample code

  The following is a sample code covering whole the qc-rpc APIs,
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
    11	  bool do_qasm = false;
    12	  char *file = NULL;
    13	  char *dir = NULL;
    14	
    15	  if (argc == 2 && strcmp(argv[1], "-s") == 0) {
    16	    do_save = true;
    17	  } else if (argc == 2 && strcmp(argv[1], "-l") == 0) {
    18	    do_load = true;
    19	  } else if (argc == 2 && strcmp(argv[1], "-re") == 0) {
    20	    do_rpc = true;
    21	  } else if (argc == 2 && strcmp(argv[1], "-rs") == 0) {
    22	    do_job_submit = true;
    23	    do_rpc = true;
    24	  } else if (argc == 4 && strcmp(argv[1], "-rq") == 0) {
    25	    do_rpc = true;
    26	    do_qasm = true;
    27	    dir = strdup(argv[2]);
    28	    file = strdup(argv[3]);
    29	  }
    30	
    31	  if (do_rpc == true) {
    32	    QC_InitRemote(&argc, &argv);
    33	  }
    34	  
    35	  QC_Init(&argc, &argv, qubits, 0); // 0 = qulacs
    36	
    37	  if (do_qasm == false) {
    38	    if (do_load == false) {
    39	
    40	      HGate(0);
    41	      U1Gate(0.1, 0);
    42	      U2Gate(0.1, 0.2, 1);
    43	      U3Gate(0.1, 0.2, 0.3, 2);
    44	
    45	      if (do_save == true) {
    46	        QC_SaveContext("test.dump");
    47	      } else if (do_rpc == true) {
    48	        QC_MeasureRemote(do_job_submit);
    49	      }
    50	    } else {
    51	      QC_LoadContext("test.dump");
    52	    }
    53	
    54	    if (do_save == false && do_rpc == false) {
    55	      QC_Measure();
    56	    }
    57	  } else {
    58	    if (dir != NULL && *dir != '\0' &&
    59	        dir != NULL && *dir != '\0') {
    60	      int max_shots = 1000;
    61	      int idx[max_shots];
    62	      double n_cnts[max_shots];
    63	      int n_shots = 0;
    64	      int i;
    65	
    66	      QC_MeasureRemoteQASMFile(dir, file,
    67				       idx, n_cnts, max_shots, &n_shots);
    68	
    69	      for (i = 0; i < n_shots; i++) {
    70	        fprintf(stdout, "%4d: %4d\t%8.4f\n", i, idx[i], n_cnts[i]);
    71	      }
    72	      
    73	    } else {
    74	      fprintf(stderr, "error: -rq flag requires work directory and "
    75	              "QASM filename under the work directory.\n");
    76	    }
    77	  }
    78	
    79	  QC_Finalize();
    80	
    81	  return 0;
    82	}
----------------------------------------------------------------

  In line 32, QC_InitRemote() is called to initialize OmniRPC
layer. Call this if you need QC programs run on an RPC server (w/
slurm.) The call must be the first function call of the application.

  In line 35, QC_Init() is called to initialize QC_API layer. All the
applications must call this function before using QC_APIs other than
QC_InitRemote().

  In lines 46 and 51, QC_{Save|Load}Context() is used to save/restore
the QC application circuit configuration information to/from a
file. These are mainly used on an RPC server/slurm side.

  In line 48, QC_MeasureRemote() starts the simulation (measurement)
with given circuit configuratioon. The do_job_submit argiment controls
how the simuration runs via, '1' for via slurm, '0' for OmniRPC REX
direct execution.

  In line 55, contrary to QC_MeasureRemote(), QC_Measure() starts the
measurement on the local machine.

  In line 66, QC_MeasureRemoteQASMFile() is brought for a QASM program
execution for RQC/QURI environment. It takes a temporary directory, a
QASM file, a qbit-pattern index array (for output,) a counts array
(also for output,) the size of these two arrays, and a number of total
shots (for output.) The QASM file and QURI/riqu-specific parameter
file(s) must be placed in the working directory. For more details, see
also qc-riqu-wrapper.sh/run_riqu.py description
(qcrex/README-run_riqu.txt.)

  Finally in line 79, QC_Finalize() finalizes the QC_API layer.

3. Compiling applications

  All the needed headers are installed in ${prefix}/include, and all
the required libraries are installed in ${prefix}/lib directory. Link
libqcs.so, libomrpc_client.so, and libomrpc_io.so to the application.

e.g.
	LIBDIR	=	$(PREFIX)/lib
	CPPFLAGS	+=	-I$(PREFIX)/include
	LDFLAGS += -Wl,-rpath -Wl,$(LIBDIR) -L$(LIBDIR} \
		-lqcs -lomprpc_clinet -lomprpc_io
