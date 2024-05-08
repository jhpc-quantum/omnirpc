# run_riqu Setup

## Install
1.Login to Fugaku
2.Run Python setup
```
$ export PYTHONUSERBASE=<path to data directory>/.local_x86_64
$ . /vol0004/apps/oss/spack/share/spack/setup-env.sh
$ spack load python@3.10.8/yt6afcn
$ spack load py-pip@23.0/wyuv6uh
```

3.Install QURI Parts riqu
```
$ pip3 install quri-parts-riqu -U --user
```

## Run
```
$ python3 run_riqu.py -d <path to data directory> -q <QASM file name> [-w <Result file name>]
```

# Setup to run from OmniRPC
## Prerequisites
1.run_riqu setup must be completed.

## Setup
1.Create a configuration file for QURI Parts riqu
```
$ vi <path to data directory>/riqu.conf
```
```
[default]
url=<base URL>
api_token=<API token>
```

2.Create QASM files
```
$ vi <path to data directory>/<QASM file name>
```
e.g.
```
OPENQASM 3;
include "stdgates.inc";
qubit[2] q;

h q[0];
cx q[0], q[1];
```
