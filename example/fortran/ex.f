      double precision res(10)
      double precision x
      integer ireqs(10)
      call omnirpc_init
      x = 0.0
      do i = 1, 10
         call omnirpc_call_async(ireqs(i),'calc_sin*',x,res(i))
         x = x + 1.0
      end do
      call omnirpc_wait_all(10,ireqs)
      do i = 1, 10
         write(*,*) 'res(',i, ')=',res(i)
      end do 
      call omnirpc_finalize
      end
