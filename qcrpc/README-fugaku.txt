A Brief Description of QC-RPC Usage on the Supercomputer Fugaku

0. Prerequisites

a) Fugaku-Specific Rules and Requirements

To install and execute software on Fugaku, it is essential to
designate the destination as the /data/<gid>/<uid>/ directory. You
should create subdirectories under this path for your convenience.

In the Fugaku QC-RPC environment, computational nodes always initiate
Remote Procedure Calls (RPCs), and login nodes accept them. This is
because computational nodes are unable to accept online remote job
execution methods such as SSH or RSH.

b) Installing QURI Parts Riqu for RQC/QURI Support on Fugaku

To execute jobs in the RQC/QURI environment on Fugaku, you must
install QURI Parts Riqu. This installation requires the use of spack
and pip3. Users are required to set the PYTHONUSERENV environment
variable before proceeding with the installation. Follow these steps
on a Fugaku login node:

---------------------------------------------------------------------------
$ export PYTHONUSERBASE=/data/<gid>/<uid>/somewhere
$ . /vol0004/apps/oss/spack/share/spack/setup-env.sh
$ spack load python@3.10.8/yt6afcn
$ spack load py-pip@23.0/wyuv6uh
$ pip3 install quri-parts-riqu -U --user
---------------------------------------------------------------------------

This installation is only necessary once on a login node since QURI
Parts Riqu is needed exclusively on the QC-RPC server side. It's
advisable to remember the PYTHONUSERBASE value, as it will be required
during the QC-RPC installation configuration.

c) Building Qulacs

The Qulacs library is a prerequisite for any environment, including
Fugaku. In most cases, the easiest way to build this system is to run
./script/build_gcc.sh within the cloned Git directory. Qulacs also
relies on the Boost library, which can be managed using the spack
utility on Fugaku, or you may choose to build Boost manually.


1. Build

In the top directory of the source tree, run the configuration script:

----------------------------------------------------------------------
$ ./configure --prefix=/where/to/install \
  --enable-qcrpc \
  --enable-homeshare \
  --with-quri=/data/<gid>/<uid>/somewhere \
  --with-qulacs=/where/the/qulacs/installed
----------------------------------------------------------------------

Use the following command to compile and install the software:

----------------------------------------------------------------------
$ make && make install
----------------------------------------------------------------------

The --enable-homeshare option is used to separate ~/.omrpc_registry by
host architectures by appending the uname -i output to the registry
directory name. For example, it will create directories like
~/.omrpc_registry.aarch64. This feature is particularly useful on
Fugaku or in other environments where user home directories are shared
across different architecture machines.

To enable RQC/QURI support, specify the --with-quri option with the
value used as PYTHONUSERBASE in the previous description, which
corresponds to the QURI Parts Riqu installation directory.

It's important to note that both the build and installation processes
must be performed separately on both a computational node and a login
node, each with different --prefix values.


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

Line 32: In line 32, the QC_InitRemote() function is called to
initialize the OmniRPC layer. You should use this if you intend to run
QC programs on an RPC server, particularly in conjunction with
Slurm. This function must be the first function call within your
application.

Line 35: Moving on to line 35, the QC_Init() function is invoked to
initialize the QC_API layer. All applications should call this
function before utilizing QC_APIs, except for QC_InitRemote().

Lines 46 and 51: Lines 46 and 51 demonstrate the use of
QC_{Save|Load}Context(). These functions are employed to save and
restore the configuration information of the QC application circuit to
and from a file. They are primarily utilized in the context of an RPC
server or Slurm environment.

Line 48: Line 48 introduces QC_MeasureRemote(), which initiates the
simulation (measurement) with a specified circuit configuration. The
do_job_submit argument dictates how the simulation is executed, with
'1' indicating execution via Slurm and '0' indicating direct execution
through OmniRPC REX.

Line 55: In contrast to QC_MeasureRemote(), line 55 references
QC_Measure(), which commences the measurement on the local machine.

Line 66: Line 66 introduces QC_MeasureRemoteQASMFile(), designed for
executing QASM programs in the RQC/QURI environment. It requires
various inputs, including a temporary directory, a QASM file, index
arrays for qubit patterns (for output), count arrays (also for
output), the sizes of these arrays, and the total number of shots (for
output). Ensure that the QASM file and any QURI/Riqu-specific
parameter files are placed in the working directory. For more detailed
information, please refer to the descriptions in qc-riqu-wrapper.sh
and run_riqu.py (found in qcrex/README-run_riqu.txt).

Line 79: Finally, in line 79, the QC_Finalize() function is used to
conclude the QC_API layer.

3. Testing RQC/QURI Support

After the installation, you can test the functionality of RQC/QURI
support on Fugaku by following these steps on the login node:

-----------------------------------------------------------------------
$ %prefix%/bin/omrpc-register --register \
  %prefix%/sbin/qcrex_qasm.rex
-----------------------------------------------------------------------

Next, edit the ~/.omrpc_registry.aarch64/hosts.xml file as follows:

-----------------------------------------------------------------------
<OmniRpcConfig>
  <Host name="login3">
    <Agent invoker="ssh" path="%install_prefix_for_login_node%"/>
  </Host>
</OmniRpcConfig>
-----------------------------------------------------------------------

Replace %install_prefix_for_login_node% with the directory specified
as --prefix for the login node configuration. "login3" is an RPC
server, and you can change this hostname to your preference. Then,
create a working directory under /data/<gid>/<uid>, and set up the
necessary files as mentioned in qcrex/README-run_riqu.txt within the
working directory.

Now, launch an interactive session on a computational node and
navigate to the top directory of the OmniRPC source tree. Then,
execute the following commands:

-----------------------------------------------------------------------
$ cd qcrpc/QC_API
$ ./apitest -rq %working_dir% %qasm_file%
-----------------------------------------------------------------------

Replace %working_dir% with the actual working directory, and
%qasm_file% with the path to a QASM source file located within the
working directory.

4. Compiling Applications

All the necessary headers are installed in ${prefix}/include, and the
required libraries are located in ${prefix}/lib directory. To link
your application successfully, you should include libqcs.so,
libomrpc_client.so, and libomrpc_io.so.

For example, in Makefile:

-----------------------------------------------------------------------
LIBDIR = $(PREFIX)/lib
CPPFLAGS += -I$(PREFIX)/include
LDFLAGS += -Wl,-rpath -Wl,$(LIBDIR) -L$(LIBDIR) \
           -lqcs -lomrpc_client -lomrpc_io
-----------------------------------------------------------------------

Ensure that your Makefile or build configuration includes these
settings for successful compilation of your applications.
