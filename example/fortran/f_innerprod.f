      subroutine innerprod(n,a,b,r)
      integer n
      double precision a(*),b(*),r
      integer i
      r = 0.0
      do i = 1,n
        r = r+a(i)*b(i)
      end do
      return
      end
