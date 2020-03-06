      double precision a(10),b(10),r,rr
      call omnirpc_init
      rr = 0.0
      do i = 1,10
         a(i) = i
         b(i) = i+10
         rr = rr + a(i)*b(i)
      end do
      call omnirpc_call('f_innerprod*',10,a,b,r)
      write(*,*) 'result=',r, rr
      end
