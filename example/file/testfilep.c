#include <OmniRpc.h>
#include <stdio.h>

int main(int argc,char *argv[])
{

    char *infile[2] = {"testfilep.c", "testfilep.c"};
    char *outfile;
    char tmp[128];
    FILE *ifp[2];
    FILE *ofp;
    FILE *fp;
    int i;

    OmniRpcInit(&argc,&argv);

    for(i = 0; i < 2; i++){
        if((ifp[i] = fopen(infile[i], "r+")) == NULL){
            perror("cannot open file");
            exit(1);
        }
    }

   if((ofp = fopen("sample_p.out", "w+")) == NULL){
       perror("cannot open file");
       exit(1);
   }

   for(i = 0; i < 2; i++){
       fprintf(stderr, "infp[%d]: FILEPOINTER: %p\n",i, ifp[i]);
       fprintf(stderr, "infp[%d]: FILENAME: %s\n",i, infile[i]);
   }
   fprintf(stderr, "outfp: FILEPOINTER: %p\n", ofp);

   OmniRpcCall("testfilep", ifp, &ofp);

   fprintf(stderr, "infp: FILEPOINTER: %p\n", ifp);
   fprintf(stderr, "outfp: FILEPOINTER: %p\n", ofp);

   while(fgets(tmp, sizeof(tmp), ofp) != NULL){
       fprintf(stdout, "OUTPUT : %s", tmp);
       fflush(stdout);
   }
   if(fclose(ofp) == EOF){
       perror("cannot close file");
   }

  OmniRpcFinalize();
}
