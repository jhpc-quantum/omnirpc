#include <OmniRpc.h>
#include <stdio.h>
#define MAX 200
int main(int argc,char *argv[])
{
   int i;
   double x, res[MAX];
	 OmniRpcRequest req[MAX];

   OmniRpcInit(&argc,&argv);

   for(i = 0; i < MAX; i++){
      req[i] = OmniRpcCallAsync("calc_sin",(double)i,&res[i]);
   }
   OmniRpcWaitAll(MAX,req);

   for(i = 0; i < MAX; i++)
      printf("sin(%d)=%g\n",i,res[i]);

   for(i = 0; i < MAX; i++){
      req[i] = OmniRpcCallAsync("calc_cos",(double)i,&res[i]);
   }
   OmniRpcWaitAll(MAX,req);

   for(i = 0; i < MAX; i++)
      printf("cos(%d)=%g\n",i,res[i]);

  OmniRpcFinalize();
}

  
