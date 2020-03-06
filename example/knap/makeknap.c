
/*   N  - number of candidate items
*    P  - capacity of knapsack as percentage of total weights generated = 80
*    Wo - minimal weight of an object = 3N
*    Wv - variance in weight of objects = N
*      -> max weight an object = 4N
*    Po - minimal profit of an object = 3N
*    Pv - variance in profit of objects = N
*    S  - initial random seed
*
* The arrays are sorted with highest profit to weight ratio first.
**************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* from Numerical Recipes: The Art of Scientific Computing */

#define M 714025
#define IA 1366
#define IC 150889

float ran2(idum)
long *idum;
{
      static long iy,ir[98];
      static int iff=0;
      int j;
      void nrerror();

      if (*idum < 0 || iff == 0) {
              iff=1;
              if ((*idum=(IC-(*idum)) % M) < 0) *idum = -(*idum);
              for (j=1;j<=97;j++) {
                      *idum=(IA*(*idum)+IC) % M;
                      ir[j]=(*idum);
              }
              *idum=(IA*(*idum)+IC) % M;
              iy=(*idum);
    }
      j=1 + 97.0*iy/M;
      if (j > 97 || j < 1) exit(1);
      iy=ir[j];
      *idum=(IA*(*idum)+IC) % M;
      ir[j]=(*idum);
      return (float) iy/M;
}

int GenRand(off,var,seed)
int off;
int var;
long *seed;
{
   int temp;
   temp = off+ (int) (ran2(seed) * var);
   return temp;
}

void print_C_W_array(A,n)
int *A;
int n;
{   int i;   for(i=0;i<n;i++) printf("W[%d] = %d;\n",(i),A[i]);
}

void print_C_P_array(A,n)
int *A;
int n;
{
   int i;
   for(i=0;i<n;i++) printf("P[%d] = %d;\n",(i),A[i]);
}

void swap_PW(P,W,i,j)
int *P;
int *W;
int i;
int j;
{
   int temp;
   temp = P[i];
   P[i] = P[j];
   P[j] = temp;
   temp = W[i];
   W[i] = W[j];
   W[j] = temp;
}

void sort_PW(P,W,n)
int *P;
int *W;
int n;
{
   int i,j;
   float maxPW = 0.0;
   int maxPWindex;

   for(j=0;j<n;j++){
      for(i=j;i<n;i++){
         if( (float)P[i]/(float)W[i] > maxPW){
            maxPW = (float)P[i]/(float)W[i];
            maxPWindex = i;
         }
      }
      swap_PW(P,W,maxPWindex,j);
      maxPW = 0.0;
   }
}

void main(argc,argv)
int argc;
char **argv;

{
   int n;
   int perc_parm;
   int Woff;
   int Wvar;
   int Poff;
   int Pvar;


   int *P;
   int *W;
   int i;
   int total_weight=0;
   float perc;

   long initial_value;
   long *seed;
   if(argc != 4) { printf("%s n seed var\n",argv[0]); exit(0); }

   sscanf(argv[1],"%d",&n);
   sscanf(argv[2],"%d",&initial_value);
   sscanf(argv[3],"%d",&Pvar);

   perc = 0.8;
   Woff = 3*n; Wvar = n; Poff = 3*n; /* Pvar = n ; */
   seed = &initial_value;
   P = (int *)malloc(sizeof(int)*n);
   W = (int *)malloc(sizeof(int)*n);

   for(i=0;i<n;i++){
      W[i] = GenRand(Woff,Wvar,seed);
      P[i] = GenRand(Poff,Pvar,seed);
      total_weight+=W[i];
   }

   sort_PW(P,W,n);

#ifdef not
   printf("n = %d;\n",n);
   printf("Cap = %d;\n",(int)((float)total_weight * perc));
   print_C_W_array(W,n);
   printf("\n");
   print_C_P_array(P,n);
   printf("\n");
#else
   printf("%d %d\n",n,(int)((float)total_weight * perc));
   for(i = 0; i < n; i++)
     printf("%d\n",W[i]);
   for(i = 0; i < n; i++)
     printf("%d\n",P[i]);
#endif

   free(P);
   free(W);
}

