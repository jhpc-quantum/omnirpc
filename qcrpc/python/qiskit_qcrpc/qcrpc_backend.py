import abc
from qiskit.providers import BackendV1 as BaseBackend
from qiskit.providers import Options

import qcrpc

class QCRPCBackend(BaseBackend):

    def _default_options(cls):
        return Options(
            shots=1024,
            job_settings=None,
            error_mitigation=None,
            extra_query_params={},
            extra_metadata={},
        )
    
    def __init__(self, configuration=None, provider=None, **kwargs):
        super().__init__(configuration=configuration, provider=provider)

    def run(self, circuit, **kwargs):
        return self._run_qasm(circuit, **kwargs)

    def _run_qasm(circuit, **kwargs):
        qasm = circuit.qasm()
        return qcrpc.submit_qasm(qasm=qasm, **kwargs)
