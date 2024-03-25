import os
import qiskit
import qiskit_qcrpc
import qiskit_qcrpc.qcrpc_provider

from qiskit import QuantumCircuit

# create a QCRPCProvider
p = qiskit_qcrpc.qcrpc_provider.QCRPCProvider()

# fetch a backend
b = p.get_backend()

# environmental variable QCRPC_URL and QCRPC_TOKEN are used implicitly
# in the backend, like belows:
#
#url = os.environ['QCRPC_URL']
#token = os.environ['QCRPC_TOKEN']
#b.set_options(url=url)
#b.set_options(token=token)
#
# b.set_options() can set other options like qctype, shots, etc,. All
# the options are shown by calling b.options.

b.set_options(transpiler=2)

qc = QuantumCircuit(2, 2)
qc.h(0)
qc.cx(0, 1)
qc.measure([0, 1], [0, 1])

b.run(qc)
