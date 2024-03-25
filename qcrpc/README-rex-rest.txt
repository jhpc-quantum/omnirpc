A Brief Description of QC-RPC Usage on the Supercomputer Fugaku
(QC-RPC REST-based REX version)

0. Prerequisites

a) Fugaku-Specific Rules and Requirements

To install and execute software on Fugaku, it is essential to
designate the destination as the /data/<gid>/<uid>/ directory. You
should create subdirectories under this path for your convenience.

In the Fugaku QC-RPC environment, computational nodes always initiate
Remote Procedure Calls (RPCs), and login nodes accept them. This is
because computational nodes are unable to accept online remote job
execution methods such as SSH or RSH.

Alod you need to load essencial packages needed to support the
REST-based REX verison of QC-RPC by using spack.

On Login node:
	$ spack load gcc@12.2.0%gcc@8.4.1/bjidm56
	$ spack load python@3.10.8%gcc@12.2.0/a5u7uck
	$ spack load py-pip@23.0%gcc@12.2.0/wyuv6uh
	$ spack load boost@1.80.0%gcc@12.2.0/lwfkszd
On Computational node:
	$ spack load gcc@12.2.0%gcc@8.5.0/sxcx7km
	$ spack load python@3.10.8%gcc@12.2.0/zcvnqre
	$ spack load py-pip@23.0%fj@4.10.0/tgxupp6
	$ spack load boost@1.80.0%fj@4.8.1/5iyob6y

b) Building Qulacs

The Qulacs library is a prerequisite for any environment, including
Fugaku. In most cases, the easiest way to build this system is running
following commands under the cloned Git directory:

	$ ./script/build_gcc.sh
	$ cd build && make shared

c) Bulding Microsoft C++ REST SDK

On Fugaku, we prepared a specific patched version of the
library. Contact administrators on how to use the library.
If not on Fugaku, clone the github repository and patch following:
================================================================
diff --git a/Release/src/http/common/http_helpers.cpp
b/Release/src/http/common/
http_helpers.cpp
index 9ffbd20d..2faceb94 100644
--- a/Release/src/http/common/http_helpers.cpp
+++ b/Release/src/http/common/http_helpers.cpp
@@ -84,7 +84,7 @@ size_t
chunked_encoding::add_chunked_delimiters(_Out_writes_(b
uffer_size) uint8_
     }
     else
     {
-        char buffer[9];
+        char buffer[17];
#ifdef _WIN32
        sprintf_s(buffer, sizeof(buffer), "%8IX", bytes_read);
#else
================================================================
Then build the library with following commands:
	$ cd Reelase && mkdir build && cd build
	$ cmake -DCMAKE_INSTALL_PREFIX=<cpprest_dir> \
	  -DBUILD_TESTS=OFF -DBUILD_SAMPLES=OFF ..
	$ make install

<cpprest_dir> is used when running OmniRPC/QC-RPC configure.


1. Build

In the top directory of the source tree, run the configuration script:

----------------------------------------------------------------------
$ ./configure --prefix=/where/to/install \
  --enable-qcrpc \
  --enable-rest \
  --enable-python \
  --enable-homeshare \
  --with-qulacs=<qulacs_srcdir> \
  --with-cpprest=<cpprest_dir>
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

--enable-rest option enables REST-based REX support. --enable-python
enables Qiskit provider using REST-based REX support.
It's important to note that both the build and installation processes
must be performed separately on both a computational node and a login
node, each with different --prefix values.


2. Sample code for newly added REST-based REX API

The following is the portion of the newly added REST-based REX API,
existig on qcrpc/QC_API/main.c:
----------------------------------------------------------------

			:
			:
    64	#ifdef USE_REST
    65	#undef SHOTS
    66	#define SHOTS	1024
    67	      char *url = NULL;
    68	      char *default_qasm =
    69	          "OPENQASM 3; include \"stdgates.inc\"; "
    70	          "qreg q[4]; creg c[4]; "
    71	          "h q[0]; h q[1]; ccx q[0], q[1], q[2]; cx q[0], q[3]; cx q[1],q[3]; "
    72	          "measure q[3] -> c[0]; measure q[2] -> c[1]; "
    73	          "measure q[1] -> c[2]; measure q[0] -> c[3]; ";
    74	      char *qasm = NULL;
    75	      char *token = NULL;
    76	      char *rem = NULL;
    77	      int qc_type = 0;
    78	      int shots = SHOTS;
    79	      int poll_ms = 1000;
    80	      int poll_max = 180;
    81	      int transpiler = 2;
    82	      int pattern[SHOTS];
    83	      float count[SHOTS];
    84	      int n_patterns = -INT_MAX;
    85	      int i;
    86	      int tmp;
    87	
    88	      for (i = 2; i < argc; i++) {
    89	        if (strcmp(argv[i], "-url") == 0) {
    90	          i++;
    91	          url = argv[i];
    92	        } else if (strcmp(argv[i], "-token") == 0) {
    93	          i++;
    94	          token = argv[i];
    95	        } else if (strcmp(argv[i], "-qasm") == 0) {
    96	          i++;
    97	          qasm = argv[i];
    98		} else if (strcmp(argv[i], "-qctype") == 0) {
    99	          i++;
   100	          tmp = atoi(argv[i]);
   101	          if (tmp >= 0) {
   102	            qc_type = tmp;
   103	          }
   104	        } else if (strcmp(argv[i], "-rem") == 0) {
   105	          i++;
   106	          rem = argv[i];
   107	        } else if (strcmp(argv[i], "-shots") == 0) {
   108	          i++;
   109	          tmp = atoi(argv[i]);
   110	          if (tmp < 1024 && tmp > 0) {
   111	            shots = tmp;
   112	          }
   113	        } else if (strcmp(argv[i], "-poll-interval") == 0) {
   114	          i++;
   115	          tmp = atoi(argv[i]);
   116	          if (tmp > 0) {
   117	            poll_ms = tmp;
   118	          }
   119	        } else if (strcmp(argv[i], "-poll-max") == 0) {
   120	          i++;
   121	          tmp = atoi(argv[i]);
   122	          if (tmp > 0) {
   123	            poll_max = tmp;
   124	          }
   125	        } else if (strcmp(argv[i], "-transpiler") == 0) {
   126	          i++;
   127	          tmp = atoi(argv[i]);
   128	          if (tmp >= 0) {
   129	            transpiler = tmp;
   130	          }
   131	        }
   132	      }
   133	
   134	      if (qasm == NULL) {
   135	        qasm = default_qasm;
   136	      }
   137	
   138	      if (url != NULL && *url != '\0' &&
   139	          token != NULL && *token != '\0' &&
   140	          qasm != NULL && *qasm != '\0') {
   141	        QC_MeasureRemoteQASMStringRESTArray(url, token, qasm, qc_type, rem,
   142	                                            shots, poll_ms, poll_max,
   143	                                            transpiler,
   144	                                            pattern, count, &n_patterns);
   145	        if (n_patterns > 0) {
   146	          for (i = 0; i < n_patterns; i++) {
   147	            fprintf(stdout, "%5d,\t%f\n", pattern[i], count[i]);
   148	          }
   149	        } else {
   150	          fprintf(stderr, "error: invalid result or any failures.\n");
   151	        }
   152	      } else {
   153	        fprintf(stderr, "error: -rest flags requires at leaset "
   154	                "-token option.\n");
   155	      }
   156	#undef SHOTS
   157	#else
   158	      fprintf(stderr, "the REST support not enabled.\n");
   159	#endif /* USE_REST */

			:
			:
----------------------------------------------------------------

Line 141, QC_MeasureRemoteQASMStringRESTArray() is the newly
added. With this API, you can specify a circuit described in QASM on a
specofied service URL with an API token issued by the service.


3. Quick hack for Fugaku

Since the Microsoft C++ REST SDK uses Boost in runtime, 

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
