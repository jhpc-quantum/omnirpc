#include"qcs_api.hpp"

void  (*gate_func[_NGates])(gate_info*) 
    = {add_IGate,
       add_XGate, 
       add_YGate, 
       add_ZGate,
       add_HGate,
       add_SGate,
       add_SdgGate,
       add_TGate,
       add_TdgGate,
       add_SXGate,
       add_SXdgGate,
       add_SYGate,
       add_SYdgGate,
       add_CXGate,
       add_CYGate,
       add_CZGate,
       add_SwapGate,
       add_RXGate,
       add_RYGate,
       add_RZGate,
       add_U1Gate,
       add_U2Gate,
       add_U3Gate,
       add_CRXGate,
       add_CRYGate,
       add_CRZGate,
       add_CCXGate,
};

