QC-RPC Provider for Qiskit How-To on Fugaku


1. Prerequisite

a) Load python, pip, and boost via spack
   on Login node:
	. /vol0004/apps/oss/spack/share/spack/setup-env.sh
	$ spack load python@3.10.8%gcc@12.2.0/a5u7uck
	$ spack load py-pip@23.0%gcc@12.2.0/wyuv6uh
	$ spack load boost@1.80.0%gcc@12.2.0/lwfkszd
   on Computational node:
	. /vol0004/apps/oss/spack/share/spack/setup-env.sh
	$ spack load python@3.10.8%gcc@12.2.0/zcvnqre
	$ spack load py-pip@23.0%fj@4.10.0/tgxupp6
	$ spack load boost@1.80.0%fj@4.8.1/5iyob6y

b) Create a python venv for Qiskit under /data/<gid>/<uid>
	$ cd /data/<gid>/<uid>
	$ mkdir python-venv.<arch> && cd python-venv.<arch>
		... where <arch> is `uname -s`
	$ python -m venv qiskit

c) Enter/Activate the venv and install qiskit
	$ . /data/<gid>/<uid>/python-venv.<arch>/qiskit/bin/activate
	(qiskit) $ pip install qiskit

  From now on, whenever you need to use QC-RPC with Qiskit, you should
run spack mentioned in a) and activate 'qiskit' venv to obey Python
package scheme. Note that when you activate 'qiskit' venv, your prompt
changes from "$" to "(qiskit) $."

d) Refer to qcrpc/README-rex-rest.txt, setup REST-enabled REX based
   QC-RPC.


2. Running a sample code

a) Acquire a QURI/IBM Q service URLs and API tokens
  Both QURI and IBM Q are not open QC environments, so acquiring
service URLs and API tokens might be restricted to registered users
only. Contact administrators.

b) Store the URL and the token in environmental variables
	(qiskit) $ export QCRPC_URL=<service URL>
	(qiskit) $ export QCRPC_TOKEN=<API token>

c) Run the sample
	(qiskit) $ cd qcrpc/python
	(qiskit) $ python ./sample.py

  The following is a result output sample, 2 qubit Bell states, run on
a Fugaku login node using QURI REST API:
---------------------------------------------------------------------
{"job_id": "49c153e3-403c-4006-ab6a-6a3f09af687f"}
{"id":"49c153e3-403c-4006-ab6a-6a3f09af687f","qasm":"OPENQASM 3.0;\ninclude \"stdgates.inc\";\nbit[2] c;\nqubit[2] q;\nh q[0];\ncx q[0], q[1];\nc[0] = measure q[0];\nc[1] = measure q[1];\n","shots":1024,"transpiler":"normal","status":"queued","result":"","transpiled_qasm":"","remark":"","in_queue":"2024-03-25 16:48:59","out_queue":"","created":"2024-03-25 16:48:59","ended":""}
Calculate Wait: 0
{"id":"49c153e3-403c-4006-ab6a-6a3f09af687f","qasm":"OPENQASM 3.0;\ninclude \"stdgates.inc\";\nbit[2] c;\nqubit[2] q;\nh q[0];\ncx q[0], q[1];\nc[0] = measure q[0];\nc[1] = measure q[1];\n","shots":1024,"transpiler":"normal","status":"preprocessing","result":"","transpiled_qasm":"","remark":"","in_queue":"2024-03-25 16:48:59","out_queue":"2024-03-25 07:49:01","created":"2024-03-25 16:48:59","ended":""}
Calculate Wait: 1
{"id":"49c153e3-403c-4006-ab6a-6a3f09af687f","qasm":"OPENQASM 3.0;\ninclude \"stdgates.inc\";\nbit[2] c;\nqubit[2] q;\nh q[0];\ncx q[0], q[1];\nc[0] = measure q[0];\nc[1] = measure q[1];\n","shots":1024,"transpiler":"normal","status":"preprocessing","result":"","transpiled_qasm":"","remark":"","in_queue":"2024-03-25 16:48:59","out_queue":"2024-03-25 07:49:01","created":"2024-03-25 16:48:59","ended":""}
Calculate Wait: 2
{"id":"49c153e3-403c-4006-ab6a-6a3f09af687f","qasm":"OPENQASM 3.0;\ninclude \"stdgates.inc\";\nbit[2] c;\nqubit[2] q;\nh q[0];\ncx q[0], q[1];\nc[0] = measure q[0];\nc[1] = measure q[1];\n","shots":1024,"transpiler":"normal","status":"success","result":"{\n  \"counts\": {\n    \"00\": 528,\n    \"11\": 496\n  },\n  \"properties\": {\n    \"0\": {\n      \"qubit_index\": 0,\n      \"measurement_window_index\": 0\n    },\n    \"1\": {\n      \"qubit_index\": 1,\n      \"measurement_window_index\": 0\n    }\n  },\n  \"transpiler_info\": {\n    \"physical_virtual_mapping\": {\n      \"0\": 0,\n      \"1\": 1\n    }\n  },\n  \"message\": \"SUCCESS!\"\n}\n","transpiled_qasm":"// Mapped to device \"wako\"\n// Qubits: 64\n// Layout (physical --\u003e virtual):\n// \tq[0] --\u003e q[0]\n// \tq[1] --\u003e q[1]\n// \tq[2] --\u003e \n// \tq[3] --\u003e \n// \tq[4] --\u003e \n// \tq[5] --\u003e \n// \tq[6] --\u003e \n// \tq[7] --\u003e \n// \tq[8] --\u003e \n// \tq[9] --\u003e \n// \tq[10] --\u003e \n// \tq[11] --\u003e \n// \tq[12] --\u003e \n// \tq[13] --\u003e \n// \tq[14] --\u003e \n// \tq[15] --\u003e \n// \tq[16] --\u003e \n// \tq[17] --\u003e \n// \tq[18] --\u003e \n// \tq[19] --\u003e \n// \tq[20] --\u003e \n// \tq[21] --\u003e \n// \tq[22] --\u003e \n// \tq[23] --\u003e \n// \tq[24] --\u003e \n// \tq[25] --\u003e \n// \tq[26] --\u003e \n// \tq[27] --\u003e \n// \tq[28] --\u003e \n// \tq[29] --\u003e \n// \tq[30] --\u003e \n// \tq[31] --\u003e \n// \tq[32] --\u003e \n// \tq[33] --\u003e \n// \tq[34] --\u003e \n// \tq[35] --\u003e \n// \tq[36] --\u003e \n// \tq[37] --\u003e \n// \tq[38] --\u003e \n// \tq[39] --\u003e \n// \tq[40] --\u003e \n// \tq[41] --\u003e \n// \tq[42] --\u003e \n// \tq[43] --\u003e \n// \tq[44] --\u003e \n// \tq[45] --\u003e \n// \tq[46] --\u003e \n// \tq[47] --\u003e \n// \tq[48] --\u003e \n// \tq[49] --\u003e \n// \tq[50] --\u003e \n// \tq[51] --\u003e \n// \tq[52] --\u003e \n// \tq[53] --\u003e \n// \tq[54] --\u003e \n// \tq[55] --\u003e \n// \tq[56] --\u003e \n// \tq[57] --\u003e \n// \tq[58] --\u003e \n// \tq[59] --\u003e \n// \tq[60] --\u003e \n// \tq[61] --\u003e \n// \tq[62] --\u003e \n// \tq[63] --\u003e \nOPENQASM 3.0;\ninclude \"stdgates.inc\";\nqreg q[64];\ncreg c[64];\nrz(1.5707963267948932) q[0];\nsx q[0];\nrz(1.5707963267948966) q[0];\ncx q[0],q[1];\nc[0] = measure q[0];\nc[1] = measure q[1];\n\n","remark":"","in_queue":"2024-03-25 16:48:59","out_queue":"2024-03-25 07:49:01","created":"2024-03-25 16:48:59","ended":"2024-03-25 07:49:02"}
read: Success
read: No child processes
---------------------------------------------------------------------
