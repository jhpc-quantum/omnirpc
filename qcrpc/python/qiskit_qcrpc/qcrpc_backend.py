import os
import abc
import qiskit.qasm3

from qiskit.providers import BackendV1 as BaseBackend
from qiskit.providers import Options

import qcrpc

class QCRPCBackend(BaseBackend):

    def _default_options(cls):
        return Options(
            qasm='',
            url='',
            token='',
            remark='',
            shots=1024,
            qctype=0,
            poll_interval=1000,
            poll_count_max=180,
            transpiler=0,
            job_settings=None,
            error_mitigation=None,
            extra_query_params={},
            extra_metadata={},
        )
    
    def __init__(self, configuration=None, provider=None, **kwargs):
        super().__init__(configuration=configuration, provider=provider)

    def run(self, circuit):
        if (circuit is not None):
            qasm = qiskit.qasm3.dumps(circuit)
        else:
            qasm = self.options['qasm']
        url = self.options['url']
        if (url is None) or (url == ''):
            url = os.environ['QCRPC_URL']
        token = self.options['token']
        if (token is None) or (token == ''):
            token = os.environ['QCRPC_TOKEN']
        qctype = self.options['qctype']
        remark = self.options['remark']
        shots = self.options['shots']
        poll_interval = self.options['poll_interval']
        poll_count_max = self.options['poll_count_max']
        transpiler = self.options['transpiler']
        return qcrpc.submit_qasm(url=url,
                                 token=token,
                                 qasm=qasm,
                                 type=qctype,
                                 remark=remark,
                                 shots=shots,
                                 poll_interval=poll_interval,
                                 poll_count_max=poll_count_max,
                                 transpile=transpiler)
