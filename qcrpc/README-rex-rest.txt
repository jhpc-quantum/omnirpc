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

You need to load essential packages needed to support the
REST-based REX version of QC-RPC by using spack.

On Login node:
	. /vol0004/apps/oss/spack/share/spack/setup-env.sh
	$ spack load gcc@12.2.0%gcc@8.4.1/bjidm56
	$ spack load python@3.10.8%gcc@12.2.0/a5u7uck
	$ spack load py-pip@23.0%gcc@12.2.0/wyuv6uh
	$ spack load boost@1.80.0%gcc@12.2.0/lwfkszd
On Computational node:
	. /vol0004/apps/oss/spack/share/spack/setup-env.sh
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

c) Building Microsoft C++ REST SDK

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

	$ cd Release && mkdir build && cd build
	$ cmake -DCMAKE_INSTALL_PREFIX=<cpprest_dir> \
	  -DBUILD_TESTS=OFF -DBUILD_SAMPLES=OFF ..
	$ make install

<cpprest_dir> is used when running OmniRPC/QC-RPC configure.


1. Build

In the top directory of the source tree, run the configuration script:

	$ ./configure --prefix=%prefix% \
	  --enable-qcrpc \
	  --enable-rest \
	  --enable-python \
	  --enable-homeshare \
	  --with-qulacs=<qulacs_srcdir> \
	  --with-cpprest=<cpprest_dir>

Use the following command to compile and install the software:

	$ make && make install

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
existing on qcrpc/QC_API/main.c:
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
specified service URL with an API token issued by the service.

This is the prototype declaration of the API:

void
QC_MeasureRemoteQASMStringRESTArray(
	const char *url, const char *token, const char *qasm, int qc_type,
	const char *rem, int shots, int poll_ms, int poll_max, int transpiler,
	int *pattern, float *count, int *n_patterns);

Where:
url		A service URL.
token		An API token.
qasm		A quantum circuit description written in QASM.
qc_type		An enumerator to specify a quantum computer model:
			0 ... RQC
			1 ... IBM Q
rem		A job remark.
shots		A # of shots.
poll_ms		A # of job status polling interval in msec.
poll_max	A # of the maximum poll count.
transpiler	An enumerator to specify a transpiler type depending
		on the qc_type.
pattern		An array of qubit patterns that has at least one count
		measurement.
count		An array of measured counts per qubit pattern.
n_patterns	A # of the qubit pattern.

count and pattern must be provided by the caller side and they must
have at least # of shots elements.


3. Environment setup

As mentioned in a former section, Microsoft C++ REST SDK uses Boost in
runtime, so Boost must also be load by spack at runtime of REX on
Fugaku. In order to do this, following instructions are needed:

a) Run:

	$ %prefix%/bin/omrpc-register --register \
		%prefix%/sbin/qcrex_qasm_redt.rex

b) open ${HOME}/.omrpc_registry.<arch>/stubs with your favorite editor
   and you can see line like this:
----------------------------------------------------------------------
qc_qasm_rest qc_rpc_rest_qasm_string %prefix%/sbin/qcrex_qasm_rest.rex
----------------------------------------------------------------------

c) Edit the line like this:
----------------------------------------------------------------------
qc_qasm_rest qc_rpc_rest_qasm_string %prefix%/sbin/qcrex_qasm_rest.sh
----------------------------------------------------------------------
(Change .rex to .sh, that's it.)

d) Create %prefix%/sbin/qcrex_qasm_rest.sh like this:
----------------------------------------------------------------------
#!/bin/sh
. /vol0004/apps/oss/spack/share/spack/setup-env.sh && \
case `uname -i` in
    x86_64)
        spack load boost@1.80.0%gcc@12.2.0/lwfkszd ;;
    aarch64)
        spack load boost@1.80.0%fj@4.8.1/5iyob6y ;;
esac
exec %prefix%/sbin/qcrex_qasm_rest.rex ${1+"$@"}
----------------------------------------------------------------------
And
	$ chmod 755 %prefix%/sbin/qcrex_qasm_rest.sh

e) On Fugaku, REX runs only on login nodes. Edit
${HOME}/.omrpc_registry.x86_64/hosts.xml like this:
-----------------------------------------------------------------------
<OmniRpcConfig>
  <Host name="login3">
    <Agent invoker="ssh" path="%login_node_install_prefix%"/>
  </Host>
</OmniRpcConfig>
-----------------------------------------------------------------------


4. Running the sample application

This section describes how to run the sample application,
qcrpc/QC_API/apitest, which made from qcrpc/QC_API/main.c described in
section 2. This sample submits a QC job to RQC via QURI REST API.

a) Make sure you run spack scripts.

b) Acquire a flesh API token from QURI/riqu cloud frontend. Then store
   the token in a file, e.g. ~/quri.token.txt.

c) Make sure you already know QURI/riqu service URL. Store the URL in
   a file, e.g. ~/quri.url.txt.

d) Run:
	$ cd qcrpc/QC_API
	$ ./apitest -rest -url `cat ~/quri.url.txt` \
		-token `cat ~/quri.token.txt` -qctype 0

  You will have output like this if the job succeeded:
-----------------------------------------------------------------------
{"job_id": "473db7ac-d5c3-40fc-8a3a-b6af8251ef9c"}
{"id":"473db7ac-d5c3-40fc-8a3a-b6af8251ef9c","qasm":"OPENQASM 3; include \"stdgates.inc\"; qreg q[4]; creg c[4]; h q[0]; h q[1]; ccx q[0], q[1], q[2]; cx q[0], q[3]; cx q[1],q[3]; measure q[3] -\u003e c[0]; measure q[2] -\u003e c[1]; measure q[1] -\u003e c[2]; measure q[0] -\u003e c[3]; ","shots":1024,"transpiler":"normal","status":"queued","result":"","transpiled_qasm":"","remark":"","in_queue":"2024-03-26 10:26:47","out_queue":"","created":"2024-03-26 10:26:47","ended":""}
Calculate Wait: 0
{"id":"473db7ac-d5c3-40fc-8a3a-b6af8251ef9c","qasm":"OPENQASM 3; include \"stdgates.inc\"; qreg q[4]; creg c[4]; h q[0]; h q[1]; ccx q[0], q[1], q[2]; cx q[0], q[3]; cx q[1],q[3]; measure q[3] -\u003e c[0]; measure q[2] -\u003e c[1]; measure q[1] -\u003e c[2]; measure q[0] -\u003e c[3]; ","shots":1024,"transpiler":"normal","status":"queued","result":"","transpiled_qasm":"","remark":"","in_queue":"2024-03-26 10:26:47","out_queue":"","created":"2024-03-26 10:26:47","ended":""}
Calculate Wait: 1
{"id":"473db7ac-d5c3-40fc-8a3a-b6af8251ef9c","qasm":"OPENQASM 3; include \"stdgates.inc\"; qreg q[4]; creg c[4]; h q[0]; h q[1]; ccx q[0], q[1], q[2]; cx q[0], q[3]; cx q[1],q[3]; measure q[3] -\u003e c[0]; measure q[2] -\u003e c[1]; measure q[1] -\u003e c[2]; measure q[0] -\u003e c[3]; ","shots":1024,"transpiler":"normal","status":"queued","result":"","transpiled_qasm":"","remark":"","in_queue":"2024-03-26 10:26:47","out_queue":"","created":"2024-03-26 10:26:47","ended":""}
Calculate Wait: 2
{"id":"473db7ac-d5c3-40fc-8a3a-b6af8251ef9c","qasm":"OPENQASM 3; include \"stdgates.inc\"; qreg q[4]; creg c[4]; h q[0]; h q[1]; ccx q[0], q[1], q[2]; cx q[0], q[3]; cx q[1],q[3]; measure q[3] -\u003e c[0]; measure q[2] -\u003e c[1]; measure q[1] -\u003e c[2]; measure q[0] -\u003e c[3]; ","shots":1024,"transpiler":"normal","status":"queued","result":"","transpiled_qasm":"","remark":"","in_queue":"2024-03-26 10:26:47","out_queue":"","created":"2024-03-26 10:26:47","ended":""}
Calculate Wait: 3
{"id":"473db7ac-d5c3-40fc-8a3a-b6af8251ef9c","qasm":"OPENQASM 3; include \"stdgates.inc\"; qreg q[4]; creg c[4]; h q[0]; h q[1]; ccx q[0], q[1], q[2]; cx q[0], q[3]; cx q[1],q[3]; measure q[3] -\u003e c[0]; measure q[2] -\u003e c[1]; measure q[1] -\u003e c[2]; measure q[0] -\u003e c[3]; ","shots":1024,"transpiler":"normal","status":"preprocessing","result":"","transpiled_qasm":"","remark":"","in_queue":"2024-03-26 10:26:47","out_queue":"2024-03-26 01:26:51","created":"2024-03-26 10:26:47","ended":""}
Calculate Wait: 4
{"id":"473db7ac-d5c3-40fc-8a3a-b6af8251ef9c","qasm":"OPENQASM 3; include \"stdgates.inc\"; qreg q[4]; creg c[4]; h q[0]; h q[1]; ccx q[0], q[1], q[2]; cx q[0], q[3]; cx q[1],q[3]; measure q[3] -\u003e c[0]; measure q[2] -\u003e c[1]; measure q[1] -\u003e c[2]; measure q[0] -\u003e c[3]; ","shots":1024,"transpiler":"normal","status":"success","result":"{\n  \"counts\": {\n    \"0011\": 256,\n    \"0100\": 264,\n    \"1001\": 240,\n    \"1010\": 264\n  },\n  \"properties\": {\n    \"0\": {\n      \"qubit_index\": 0,\n      \"measurement_window_index\": 0\n    },\n    \"1\": {\n      \"qubit_index\": 1,\n      \"measurement_window_index\": 0\n    },\n    \"2\": {\n      \"qubit_index\": 2,\n      \"measurement_window_index\": 0\n    },\n    \"3\": {\n      \"qubit_index\": 3,\n      \"measurement_window_index\": 0\n    }\n  },\n  \"transpiler_info\": {\n    \"physical_virtual_mapping\": {\n      \"0\": 3,\n      \"1\": 1,\n      \"2\": 0,\n      \"3\": 2\n    }\n  },\n  \"message\": \"SUCCESS!\"\n}\n","transpiled_qasm":"// Mapped to device \"wako\"\n// Qubits: 64\n// Layout (physical --\u003e virtual):\n// \tq[0] --\u003e q[3]\n// \tq[1] --\u003e q[1]\n// \tq[2] --\u003e q[0]\n// \tq[3] --\u003e q[2]\n// \tq[4] --\u003e \n// \tq[5] --\u003e \n// \tq[6] --\u003e \n// \tq[7] --\u003e \n// \tq[8] --\u003e \n// \tq[9] --\u003e \n// \tq[10] --\u003e \n// \tq[11] --\u003e \n// \tq[12] --\u003e \n// \tq[13] --\u003e \n// \tq[14] --\u003e \n// \tq[15] --\u003e \n// \tq[16] --\u003e \n// \tq[17] --\u003e \n// \tq[18] --\u003e \n// \tq[19] --\u003e \n// \tq[20] --\u003e \n// \tq[21] --\u003e \n// \tq[22] --\u003e \n// \tq[23] --\u003e \n// \tq[24] --\u003e \n// \tq[25] --\u003e \n// \tq[26] --\u003e \n// \tq[27] --\u003e \n// \tq[28] --\u003e \n// \tq[29] --\u003e \n// \tq[30] --\u003e \n// \tq[31] --\u003e \n// \tq[32] --\u003e \n// \tq[33] --\u003e \n// \tq[34] --\u003e \n// \tq[35] --\u003e \n// \tq[36] --\u003e \n// \tq[37] --\u003e \n// \tq[38] --\u003e \n// \tq[39] --\u003e \n// \tq[40] --\u003e \n// \tq[41] --\u003e \n// \tq[42] --\u003e \n// \tq[43] --\u003e \n// \tq[44] --\u003e \n// \tq[45] --\u003e \n// \tq[46] --\u003e \n// \tq[47] --\u003e \n// \tq[48] --\u003e \n// \tq[49] --\u003e \n// \tq[50] --\u003e \n// \tq[51] --\u003e \n// \tq[52] --\u003e \n// \tq[53] --\u003e \n// \tq[54] --\u003e \n// \tq[55] --\u003e \n// \tq[56] --\u003e \n// \tq[57] --\u003e \n// \tq[58] --\u003e \n// \tq[59] --\u003e \n// \tq[60] --\u003e \n// \tq[61] --\u003e \n// \tq[62] --\u003e \n// \tq[63] --\u003e \nOPENQASM 3.0;\ninclude \"stdgates.inc\";\nqreg q[64];\ncreg c[64];\nrz(1.5707963267948921) q[3];\nsx q[3];\nrz(-2.3561944901923484) q[3];\nsx q[3];\nrz(1.5707963267948952) q[3];\ncx q[3],q[2];\nrz(-1.5707963267948986) q[3];\nsx q[3];\nrz(-2.3561944901923417) q[3];\nsx q[3];\nrz(-1.5707963267948952) q[3];\nrz(1.5707963267948921) q[1];\nsx q[1];\nrz(-2.3561944901923484) q[1];\nsx q[1];\nrz(1.5707963267948952) q[1];\ncx q[3],q[1];\nrz(1.5707963267948921) q[3];\nsx q[3];\nrz(-2.3561944901923484) q[3];\nsx q[3];\nrz(1.5707963267948952) q[3];\ncx q[3],q[2];\nrz(-1.5707963267948986) q[3];\nsx q[3];\nrz(-2.3561944901923417) q[3];\nsx q[3];\nrz(-1.5707963267948952) q[3];\ncx q[3],q[1];\nrz(1.5707963267948932) q[2];\nsx q[2];\nrz(2.3561944901923444) q[2];\ncx q[0],q[2];\nrz(1.5707963267948932) q[0];\nsx q[0];\nrz(1.5707963267948966) q[0];\ncx q[0],q[1];\nrz(1.5707963267948932) q[0];\nsx q[0];\nrz(1.5707963267948966) q[0];\ncx q[0],q[2];\nrz(1.5707963267948932) q[0];\nsx q[0];\nrz(1.5707963267948966) q[0];\nrz(0.7853981633974455) q[2];\nsx q[2];\nrz(1.5707963267948966) q[2];\ncx q[0],q[2];\nrz(1.5707963267948932) q[0];\nsx q[0];\nrz(1.5707963267948966) q[0];\nrz(1.5707963267948932) q[2];\nsx q[2];\nrz(1.5707963267948966) q[2];\ncx q[0],q[2];\nrz(1.5707963267948932) q[0];\nsx q[0];\nrz(1.5707963267948966) q[0];\ncx q[0],q[1];\nrz(1.5707963267948932) q[0];\nsx q[0];\nrz(1.5707963267948966) q[0];\ncx q[0],q[2];\nrz(1.5707963267948932) q[1];\nsx q[1];\nrz(1.5707963267948966) q[1];\nc[0] = measure q[0];\nc[3] = measure q[3];\nc[1] = measure q[1];\nc[2] = measure q[2];\n\n","remark":"","in_queue":"2024-03-26 10:26:47","out_queue":"2024-03-26 01:26:51","created":"2024-03-26 10:26:47","ended":"2024-03-26 01:26:51"}
    3,	256.000000
    4,	264.000000
    9,	240.000000
   10,	264.000000
read: Success
read: No child processes
-----------------------------------------------------------------------


5. Compiling applications

All the necessary headers are installed in ${prefix}/include, and the
required libraries are located in ${prefix}/lib directory. To link
your application successfully, you should include libqcs.so,
libomrpc_client.so, and libomrpc_io.so.

For example, in Makefile:

-----------------------------------------------------------------------
LIBDIR = $(PREFIX)/lib
CPPFLAGS += -I$(PREFIX)/include
LDFLAGS += -Wl,-rpath -Wl,$(LIBDIR) -L$(LIBDIR) \
           -lqcs -lomrpc_client -lomrpc_io -lrqcrest
-----------------------------------------------------------------------

Ensure that your Makefile or build configuration includes these
settings for successful compilation of your applications.
