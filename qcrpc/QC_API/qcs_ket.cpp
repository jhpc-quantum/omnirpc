#define _INC_QCS_KET_HPP_
#include"qcs_api.hpp"
#undef _INC_QCS_KET_HPP_
#include"qcs_gate_funcs.hpp"

#define _DEBUG_OUT_ 

#ifdef _DEBUG_OUT_
#define _DEBUG_FUNC_OUT()  printf("%s :",__func__); \
  for(int i=0; i<gate_info->niarg; i++){ \
    printf("%d ",gate_info->iarg[i]); \
  } \
  for(int i=0; i<gate_info->nrarg; i++){	\
    printf("%f ",gate_info->rarg[i]); \
  } \
  printf("\n")
#else
#define _DEBUG_FUNC_OUT() ""
#endif

ket_info *ki; 
yampi::environment *mpi_env = NULL;

extern "C" void error(std::string s, std::string fname, int n)
{
  printf("%s %s %d\n", s.c_str(), fname.c_str(), n);
  exit(1);
}

extern "C" void qcs_init_lib(qint nqubits){
  ki = new ket_info;

  int  argc    = 0;
  char **argv  = NULL;
  if(mpi_env==NULL){
    ki->environment  = new yampi::environment(argc, argv, yampi::thread_support::single);
  }else{
    ki->environment = mpi_env; // MPI_Init called elsewhere
  }
  ki->communicator = new yampi::communicator(yampi::tags::world_communicator());
  ki->rank         = ki->communicator->rank(*(ki->environment));
  ki->nprocs       = ki->communicator->size(*(ki->environment));
  MPI_Comm_rank(MPI_COMM_WORLD, &(ki->myrank));
  ki->root         = yampi::rank{0};

  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->nprocs);
  ki->nqubits = nqubits;
  auto const num_qubits = bit_integer_type{(unsigned int)nqubits};
  auto const num_lqubits = num_qubits - num_gqubits;
  //auto const initial_state_value = state_integer_type{(1u<<num_qubits)-1};  
  auto const initial_state_value = state_integer_type{0u};

  ki->permutation = new ket::mpi::qubit_permutation<state_integer_type, bit_integer_type>{num_qubits};
  ki->local_state = new ket::mpi::state<complex_type, false, yampi::allocator<complex_type>>{num_lqubits, initial_state_value, *(ki->permutation), *(ki->communicator), *(ki->environment)}; 
}

extern "C" void qcs_finalize_lib()
{
  if(ki){
    if(ki->permutation) delete ki->permutation;
    if(ki->local_state) delete ki->local_state;
    delete ki;
  }
}

extern "C" void qcs_measure(qcs_info_t* qcs_info)
{
  int n = qcs_info->ngates;
  
  // 一気に gate を add or apply する
  for(int i=0; i<n; i++){
    void (*f)(gate_info*) = gate_func[qcs_info->gate[i].id];
    f(&(qcs_info->gate[i]));
  }
  if(qcs_info->qcs_id == _braket_riken){
    printf("debug\n"); fflush(stdout);
    double *state = (double*)qcs_update();
    for(int i=0; i<8; i++){
      if(ki->rank==yampi::rank{0}){
      printf("%f %f\n",state[i*2],state[i*2+1]);
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

// デバッグ用
/*
// mempos : position in memory
// bitpos : position in state
// convert "mempos" to bitpos and return
 if perm = (0,0),(1,1),(2,3)(3,2) then
    x=0000 -> 0000
    x=0101 -> 0110
    x=0010 -> 0001
*/
uint64_t to_bitpos(uint64_t mempos, const ket::mpi::qubit_permutation<long unsigned int, unsigned int>& permutation)
{
  int myrank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  
  using qubit = ::ket::qubit<long unsigned int, unsigned int>;
  uint64_t globaln = (1L<< permutation.size());
  uint64_t localn = globaln/nprocs;
  uint64_t nglobal_bit = permutation.size();
  uint64_t nlocal_bit = nglobal_bit-(log(nprocs)/log(2));
  uint64_t bit = 0;
  uint64_t mask = 1;
  
  //print_bit(mempos);
  for(uint64_t i=0; i<nglobal_bit; i++){
    uint64_t mask = (1<<permutation[qubit{i}]); 
    if((mempos&mask)!=0){ //  if mempos = 1, then pos(i) = 1
      bit |= (1UL<<i);
    }
  }

  //ULONG_MAX
  //18446744073709551615
  
  return bit;
}

// デバッグ用の関数
extern "C" void copy_state_p(std::vector<complex_type, yampi::allocator<complex_type>> data, double *__restrict state, int num_gstates, int num_lstates, ket::mpi::qubit_permutation<state_integer_type, bit_integer_type> perm)
{
  int    myrank = ki->myrank;
  double *state1 = (double *)calloc(2*num_gstates, sizeof(double));
  unsigned int base = myrank*num_lstates;

  for(unsigned int i = myrank*num_lstates; i<(myrank+1)*num_lstates; i++){
    unsigned int bit = to_bitpos(i, perm);
    state1[2*bit]   = real(data[i-base]);
    state1[2*bit+1] = imag(data[i-base]);
  }

  MPI_Allreduce(state1, state, num_gstates*2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  free(state1);
  
} /* copy_state_p */


// 一時的な実装
extern "C" void *qcs_update()
{
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto const data = ki->local_state->data();

  double *state1 = (double *)calloc(2*num_gstates, sizeof(double));
  unsigned int n = data.size();
  unsigned int base = (ki->myrank)*num_lstates*2;

  copy_state_p(ki->local_state->data(), state1, num_gstates, num_lstates, *(ki->permutation));
  if(ki->myrank==0){
    return (void *)state1; 
  }else{
    free(state1); 
    return NULL;
  }
}

// デバッグ用
void prt_state()
{
  auto const data = ki->local_state->data();
  int n = data.size();
  for(int i=0; i<n; i++){
    std::cout << ki->myrank << " " << data[i] << std::endl;
  }
}

extern "C" void add_IGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  // I gateは何もしない
}

extern "C" void add_XGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};
  ket::mpi::gate::pauli_x(*(ki->local_state), qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_YGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};
  ket::mpi::gate::pauli_y(*(ki->local_state), qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_ZGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};
  ket::mpi::gate::pauli_z(*(ki->local_state), qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_HGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};
  ket::mpi::gate::hadamard(*(ki->local_state), qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_SGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};
  ket::mpi::gate::phase_shift(*(ki->local_state), (M_PI*0.5), qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_SdgGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};
  ket::mpi::gate::phase_shift(*(ki->local_state), (-M_PI*0.5), qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_TGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  ket::mpi::gate::phase_shift(*(ki->local_state), (M_PI*0.25), qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_TdgGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  ket::mpi::gate::phase_shift(*(ki->local_state), (-M_PI*0.25), qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_SXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  // RX(PI/2)
  ket::mpi::gate::phase_shift3(*(ki->local_state), M_PI/2.0, -M_PI/2.0, M_PI/2.0, qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
  // multiply exp(i PI/4) and the state vector
  // mult.hpp is implemented temporary 
  complex_type epi4(0.0, M_PI*0.25);
  complex_type c    = exp(epi4);
  ket::mpi::gate::mult(*(ki->local_state), c, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_SXdgGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  // RX(-PI/2)
  ket::mpi::gate::phase_shift3(*(ki->local_state), -M_PI/2.0, -M_PI/2.0, M_PI/2.0, qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[0])}}, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
  // multiply exp(i PI/4) and the state vector
  // mult.hpp is implemented temporary 
  complex_type epi4(0.0, -M_PI*0.25);
  complex_type c    = exp(epi4);
  ket::mpi::gate::mult(*(ki->local_state), c, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

// TODO
extern "C" void add_SYGate(gate_info *gate_info)
{
  error("The SYGate has not been implemented for ket ",__FILE__,__LINE__);
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }  
}

//TODO
extern "C" void add_SYdgGate(gate_info *gate_info)
{
  error("The SYGate has not been implemented for ket ",__FILE__,__LINE__);
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
}

extern "C" void add_CXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }

  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  ket::control<qubit_type> control_qubit{qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[1])}}};

  ket::mpi::gate::controlled_not(*(ki->local_state), target_qubit, control_qubit, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_CYGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  error("error: The CYGate has not supported for ket ",__FILE__,__LINE__);
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
}

extern "C" void add_CZGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  } 
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  //qcs_info->gate[n].iarg[0] = target_qubit;
  //qcs_info->gate[n].iarg[1] = control_qubit;
  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  ket::control<qubit_type> control_qubit{qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[1])}}};
 
  ket::mpi::gate::controlled_phase_shift(*(ki->local_state), M_PI, target_qubit, control_qubit, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_SwapGate(gate_info *gate_info)
{
  error("The SwapGate has not been supported for ket",__FILE__,__LINE__);
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
}

extern "C" void add_RXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  double theta = gate_info->rarg[0];

  ket::mpi::gate::phase_shift3(*(ki->local_state), theta, -M_PI/2.0, M_PI/2.0, target_qubit, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment)); 

}

extern "C" void add_RYGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  double theta = gate_info->rarg[0];

  ket::mpi::gate::phase_shift3(*(ki->local_state), theta, 0.0, 0.0, target_qubit, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment)); 

}

extern "C" void add_RZGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  double theta = gate_info->rarg[0];
  
  ket::mpi::gate::phase_shift(*(ki->local_state), theta, target_qubit, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment)); 
  complex_type c{cos(theta/2.0),-sin(theta/2.0)};
  ket::mpi::gate::mult(*(ki->local_state), c, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_U1Gate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  double theta = gate_info->rarg[0];
  
  ket::mpi::gate::phase_shift(*(ki->local_state), theta, target_qubit, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment)); 
}

extern "C" void add_U2Gate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  double theta0 = gate_info->rarg[0];
  double theta1 = gate_info->rarg[1];
  
  ket::mpi::gate::phase_shift2(*(ki->local_state), theta0, theta1, target_qubit, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));

}

extern "C" void add_U3Gate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  double theta0 = gate_info->rarg[0];
  double theta1 = gate_info->rarg[1];
  double theta2 = gate_info->rarg[1];
  
  ket::mpi::gate::phase_shift3(*(ki->local_state), theta0, theta1, theta2, target_qubit, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

extern "C" void add_CRXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  error("not implemented",__FILE__,__LINE__);
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};
  // ...??? AAA
  
}

extern "C" void add_CRYGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  error("not implemented",__FILE__,__LINE__);
}

extern "C" void add_CRZGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  error("not implemented",__FILE__,__LINE__);
}

extern "C" void add_CCXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  std::ios::sync_with_stdio(false);
  auto const num_qubits  = bit_integer_type{(unsigned int)ki->nqubits};
  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(ki->communicator->size(*(ki->environment)));
  auto const num_lqubits = num_qubits - num_gqubits;
  auto const num_gstates = (1u<<num_qubits);
  auto const num_lstates = num_gstates/(ki->nprocs); 
  auto buffer = std::vector<complex_type>{};

  //qcs_info->gate[n].iarg[0] = target_qubit;
  //qcs_info->gate[n].iarg[1] = control_qubit0;
  //qcs_info->gate[n].iarg[2] = control_qubit1;

  qubit_type target_qubit{bit_integer_type{(unsigned int)(gate_info->iarg[0])}};
  ket::control<qubit_type> control_qubit0{qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[1])}}};
  ket::control<qubit_type> control_qubit1{qubit_type{bit_integer_type{(unsigned int)(gate_info->iarg[2])}}};
  //ket::mpi::gate::toffoli(*(local_state), target_qubit, control_qubit0, control_qubit1, *(permutation), buffer, *(communicator), *(environment));


  ket::mpi::gate::toffoli(*(ki->local_state), target_qubit, control_qubit0, control_qubit1, *(ki->permutation), buffer, *(ki->communicator), *(ki->environment));
}

