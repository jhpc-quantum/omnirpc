from . import qcrpc_backend

class QCRPCProvider:
    name = "qcrpc_provider"
    backend_name = "qcrpc_backend"

    def __init__(self, **kwargs):
        super().__init__()
        self._backends = {
            self.backend_name : qcrpc_backend.QCRPCBackend(self, **kwargs)
        }

    def backends(self, name=None):
        if name:
            return self._backends[name]
        else:
            return self._backends[self.backend_name]

    def get_backend(self, name=None, **kwargs):
        return self.backends(name, **kwargs)
