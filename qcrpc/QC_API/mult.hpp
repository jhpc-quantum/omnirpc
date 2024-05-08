# include <boost/config.hpp>

# include <complex>
# include <vector>
# include <array>

# include <boost/range/value_type.hpp>

# include <yampi/environment.hpp>
# include <yampi/datatype_base.hpp>
# include <yampi/communicator.hpp>

# include <ket/qubit.hpp>
# include <ket/gate/phase_shift.hpp>
# include <ket/mpi/permutated.hpp>
# include <ket/mpi/qubit_permutation.hpp>
# include <ket/mpi/utility/general_mpi.hpp>
# include <ket/mpi/utility/for_each_local_range.hpp>
# include <ket/mpi/utility/logger.hpp>
# include <ket/mpi/gate/page/phase_shift.hpp>
# include <ket/mpi/page/is_on_page.hpp>

//    ket::mpi::gate::mult(*local_state, *coef, *permutation, buffer, *communicator, *environment);

namespace ket
{
  namespace mpi
  {
    namespace gate
    {
        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Real,
	  typename RandomAccessIterator>
	  inline void mult(
	    MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
	    Real const& coef,
	    RandomAccessIterator const first, RandomAccessIterator const last)
	{
	  using ::ket::utility::loop_n;
	  loop_n(
		 parallel_policy,
		 last-first,
		 //static_cast<StateInteger>(last - first) >> 1u,
		 [first, coef](auto const idx, int const)
		 {
		   *(first+idx) *= coef;
		 }
	     );
	}

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Real,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& do_mult(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Real const& coef,	  
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [mpi_policy, parallel_policy, coef](
              auto const first, auto const last)
            {
	      ::ket::mpi::gate::mult(mpi_policy, parallel_policy, coef, first, last);
            });
        }

      template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Real,
          typename StateInteger, typename BitInteger,	
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& mult(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Real const& coef,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator,
          yampi::environment const& environment)
        {
          return ::ket::mpi::gate::do_mult(
            mpi_policy, parallel_policy,
            local_state, coef, permutation,
            communicator, environment);
        }
      
      template <
        typename RandomAccessRange, typename Real,
	typename StateInteger, typename BitInteger,	
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& mult(
        RandomAccessRange& local_state,
        Real const coef,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator,
        yampi::environment const& environment)
      {
        return ::ket::mpi::gate::mult(
          ::ket::mpi::utility::policy::make_general_mpi(),
          ::ket::utility::policy::make_sequential(),				    
          local_state, coef, permutation, buffer, communicator, environment);
      }



      template <
          typename MpiPolicy, typename ParallelPolicy,
          typename Complex,
	  typename RandomAccessIterator>
	  inline void multc(
	    MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
	    Complex const& coef,
	    RandomAccessIterator const first, RandomAccessIterator const last)
	{
	  using ::ket::utility::loop_n;
	  loop_n(
		 parallel_policy,
		 last-first,
		 //static_cast<StateInteger>(last - first) >> 1u,
		 [first, coef](auto const idx, int const)
		 {
		   *(first+idx) *= coef;
		 }
	     );
	}

        template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger, typename Allocator>
        inline RandomAccessRange& do_multc(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& coef,	  
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          yampi::communicator const& communicator, yampi::environment const& environment)
        {
          return ::ket::mpi::utility::for_each_local_range(
            mpi_policy, local_state, communicator, environment,
            [mpi_policy, parallel_policy, coef](
              auto const first, auto const last)
            {
	      ::ket::mpi::gate::mult(mpi_policy, parallel_policy, coef, first, last);
            });
        }

      template <
          typename MpiPolicy, typename ParallelPolicy,
          typename RandomAccessRange, typename Complex,
          typename StateInteger, typename BitInteger,	
          typename Allocator, typename BufferAllocator>
        inline RandomAccessRange& multc(
          MpiPolicy const& mpi_policy, ParallelPolicy const parallel_policy,
          RandomAccessRange& local_state,
          Complex const& coef,
          ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
          std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
          yampi::communicator const& communicator,
          yampi::environment const& environment)
        {
          return ::ket::mpi::gate::do_multc(
            mpi_policy, parallel_policy,
            local_state, coef, permutation,
            communicator, environment);
        }
      
      template <
        typename RandomAccessRange, typename Complex,
	typename StateInteger, typename BitInteger,	
        typename Allocator, typename BufferAllocator>
      inline RandomAccessRange& multc(
        RandomAccessRange& local_state,
        Complex const coef,
        ::ket::mpi::qubit_permutation<StateInteger, BitInteger, Allocator>& permutation,
        std::vector<typename boost::range_value<RandomAccessRange>::type, BufferAllocator>& buffer,
        yampi::communicator const& communicator,
        yampi::environment const& environment)
      {
        return ::ket::mpi::gate::multc(
          ::ket::mpi::utility::policy::make_general_mpi(),
          ::ket::utility::policy::make_sequential(),				    
          local_state, coef, permutation, buffer, communicator, environment);
      }
    }}}
