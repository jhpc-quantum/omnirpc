static char rcsid[] = "$Id: sample-lib.c,v 1.1.1.1 2004-11-03 21:01:38 yoshihiro Exp $";
#include <stdio.h>
/* 
 * $RWC_Release$
 * $RWC_Copyright$
 */

void mmul(n,A,B,C)
    double *A,*B,*C;
{
    double t;
    int i,j,k;
    
    for (i=0;i<n;i++){
        for (j=0;j<n;j++){
            t = 0;
            for (k=0;k<n;k++){
                t += A[i*n + k] * B[k*n+j];     /* inner product */
            }
            C[i*n+j] = t;
        }
    }
}

void FFT(n,x,y)
{
    printf("no FFT sorry!\n");
}
