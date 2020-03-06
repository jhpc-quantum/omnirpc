#include <OmniRpc.h>
#include <stdio.h>

int main(int argc,char *argv[])
{

   char *infile[2] = {"testfile.c", "testfilep.c"};
   char *outfile = "sample.out";
   char tmp[128];
   FILE *fp;
   int i,j;
   int c = 0;

   OmniRpcInit(&argc,&argv);

   for(i = 0 ;i < 2; i++){
       if((fp = fopen(infile[i], "w+")) == NULL){
           perror("cannot open file");
           exit(1);
       }
       for(j =0; j < 128; j++)
           fprintf(fp, "%i\n",c++);
       fflush(fp);
       if(fclose(fp) != 0){
           perror("cannot close file");
       }
   }

   OmniRpcCall("testfile", infile, &outfile);

   fprintf(stderr, "TRANSFER FIN\n");
   fflush(stderr);
   if((fp = fopen(outfile, "r+")) == NULL){
       perror("cannot open file");
       exit(1);
   }
   while(fgets(tmp, sizeof(tmp), fp) != NULL){
       fprintf(stdout, "OUTPUT:%s", tmp);
   }

  OmniRpcFinalize();
}

