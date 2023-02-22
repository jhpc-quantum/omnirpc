/* 
 * omrpc-register-yml.c 2012.01.02 M.Tsuji
 * $Release: omnirpc-2.0.1 $
 * $Copyright:
 *  OmniRPC Version 1.0
 *  Copyright (C) 2002-2004 HPCS Laboratory, University of Tsukuba.
 *  
 *  This software is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version
 *  2.1 published by the Free Software Foundation.
 *  
 *  Please check the Copyright and License information in the files named
 *  COPYRIGHT and LICENSE under the top  directory of the OmniRPC Grid PRC 
 *  System release kit.
 *  
 */

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "omrpc_defs.h"
#include "omrpc_rpc.h"
#include "omrpc_reg_defs.h"
#include "ninf_stub_info.h"

#ifdef MYX
#define MUST_MODULE_DIR "-L/home/tsuji/local/MUST3/MUST-v1.6/modules/"
#define MUST_LIB_DIR "-L/home/tsuji/local/MUST3/MUST-v1.6/lib/"
#endif
static int  prt_log = 1;

static int  n_entries = 0;
static int  n_rexes = 0;

static char prefix_dir[MAXPATHLEN];
static char rex_dir[MAXPATHLEN];
static char lib_dir[MAXPATHLEN];
static char inc_dirn[MAXPATHLEN];
static char imp_dir[MAXPATHLEN];

int main(int argc, char **argv)
{
  int    i, j;
  uint32 fd, status, offset, state;
  uint32 name_size, data_size;
  uint32 to_read;
  char   buf[MAX_BUF2], buf2[MAX_BUF2], module_name[MAXPATHLEN], ctmp[MAXPATHLEN];
  char   name[MAX_BUF], fname[MAXPATHLEN];
  char   *ptr;
  char   f_starpu=0;

  if(argc!=2){
    omrpc_error_exit("cannot find input file\nUSAGE: omrpc-register-yml <yml-yapp-file>");
  }

  omrpc_gsub(argv[1], module_name, ".", "_");
  omrpc_set_dir();

  fd=open(argv[1], O_RDWR, 0666);
  if(fd==-1){
    omrpc_error_exit("cannot open file %s",argv[1]);
  }

  // read header 
  status=read(fd,buf,sizeof(buf));
  if(status<4){
    omrpc_error_exit("file %s is invalid",argv[1]);
  }
  if(strncmp(buf,"ARC1",4)){
    omrpc_error_exit("file %s may not be YML Application file",argv[1]);
  }

  // read body
  lseek(fd,4,SEEK_SET);
  status=1;
  state =0;

  while(status){
    offset=0;
    memset(buf,0,sizeof(buf));
    status=read(fd,buf,sizeof(buf));
    if(status==-1){
      close(fd);
      omrpc_error_exit("cannot read file %s",argv[1]);
    }
    while(status-offset>0){
      // in the name_size chunk 
      if(state<sizeof(name_size)){
        to_read=sizeof(name_size)-state;
        to_read=(status-offset>to_read) ? to_read : status-offset;
        memcpy((char *)(&name_size),buf+offset,to_read);
        offset+=to_read;
        state +=to_read;
      }
      // in the data_size chunk
      else if(state<sizeof(name_size)+sizeof(data_size)){
        to_read=sizeof(data_size)-state+sizeof(name_size);
        to_read=(status-offset>to_read) ? to_read : status-offset;
        memcpy((char *)(&data_size),buf+offset,to_read);
        offset+=to_read;
        state +=to_read;
      }
      // in the name chunk
      else if(state<sizeof(name_size)+sizeof(data_size)+name_size){
        to_read=name_size-state+sizeof(name_size)+sizeof(data_size);
        to_read=(status-offset>to_read) ? to_read : status-offset;
        memset(name,0,sizeof(name));
        memcpy(name,buf+offset,to_read);
        offset+=to_read;
        state +=to_read;
      }
      // in the data chunk
      else{
        to_read=data_size-state+sizeof(name_size)+sizeof(data_size)+name_size;
        to_read=(status-offset>to_read) ? to_read : status-offset;
        if(!strcmp(name,"data_tasks")){
          memset(buf2,0,sizeof(buf2));
          memcpy(buf2,buf+offset,to_read);
          status=0;
          break;
        }
        if(state>=sizeof(name_size)+sizeof(data_size)+name_size+data_size){
          state=0;
          name_size=0;
          data_size=0;
        }
        offset+=to_read;
        state +=to_read;
      }
    }
  }
  close(fd);

  ptr=buf2;

  while(omrpc_gets(&ptr,buf,MAX_BUF)){
    if(strstr(buf,"***")){
      omrpc_put_reg(&ptr, module_name);
    } 
  }

  // ---------- language & libraries ---------- 
  for(i=0; i<n_rexes; i++){
    if(prt_log) printf("===== %s.rex =====\n",rex_table[i].name);
    rex_table[i].n_lib=0;
    rex_table[i].n_libtxt=0;

    for(j=0; j<rex_table[i].n_entries; j++){
      sprintf(ctmp,"%s/%s.xml",imp_dir,rex_table[i].reg_ptr[j]->entry_name);
      omrpc_get_lang(ctmp,&(rex_table[i].reg_ptr[j]->lang[0])); 
      if(strcmp(rex_table[i].reg_ptr[j]->lang,"XDS")==0){ 
        f_starpu=1;
      }
      if(prt_log) printf("name=%s: lang=%s, ",rex_table[i].reg_ptr[j]->entry_name,rex_table[i].reg_ptr[j]->lang);
      omrpc_get_libtxt(ctmp, &(rex_table[i]));
      if(prt_log) printf("\n");
    }
    omrpc_get_lib(&(rex_table[i]));

    if(prt_log) printf("\n");
  }

  // ---------- idl ---------- 
  for(i=0; i<n_rexes; i++)
    omrpc_marge_idl(i);

  // ---------- idl -> rex.c -> rex.o -> rex ---------- 
  for(i=0; i<n_rexes; i++){
    if(prt_log) printf("Making %s.rex\n",rex_table[i].name);

    // idl -> rex.c 
    sprintf(buf, "omrpc-gen-xmp %s/%s.idl > %s/%s.rex.c\n",rex_dir,rex_table[i].name,rex_dir,rex_table[i].name);
    system(buf);

    // rex.c -> rex.o 
    if(f_starpu){
      sprintf(buf, "%s -c %s/%s.rex.c -o %s/%s.rex.o -I%s/include/yml -I%s -I%s/include -DXMP_DEV_STARPU\n"
             ,CC,rex_dir,rex_table[i].name,rex_dir,rex_table[i].name,prefix_dir,inc_dir,prefix_dir);
    }else{
      sprintf(buf, "%s -c %s/%s.rex.c -o %s/%s.rex.o -I%s/include/yml -I%s -I%s/include\n"
             ,CC,rex_dir,rex_table[i].name,rex_dir,rex_table[i].name,prefix_dir,inc_dir,prefix_dir);
    }
    if(prt_log) printf("%s\n",buf);
    system(buf);

    // rex.o -> rex
    memset(buf,0,sizeof(buf));
    sprintf(buf, "%s -rdynamic -o %s/%s.rex %s/%s.rex.o %s/libxmp_std_lib.a"
           ,CC,rex_dir,rex_table[i].name,rex_dir,rex_table[i].name,lib_dir);

    for(j=0; j<rex_table[i].n_entries; j++){
      memset(ctmp,0,sizeof(ctmp));
      if(strcmp(rex_table[i].reg_ptr[j]->lang,"XDS")==0){ 
        char cmd[1024];
        // replace "_XMP_STARPU_FUNC_0" with "_XMP_STARPU_FUNC_funcname_0"
        memset(fname,0,MAXPATHLEN);
        sprintf(fname,"%s/%s_dir/__omni_tmp__/%s.c",rex_dir,rex_table[i].reg_ptr[j]->entry_name,rex_table[i].reg_ptr[j]->entry_name);
        omrpc_rename_func(fname,rex_table[i].reg_ptr[j]->entry_name);
        memset(cmd,0,1024);
        sprintf(cmd,"mpicc -std=gnu99 -O3 -D_XCALABLEMP -c %s_tmp.c -o %s/%s.o -I/%s/include",fname,rex_dir,rex_table[i].reg_ptr[j]->entry_name,prefix_dir);
        if(prt_log) printf("%s\n",cmd);
        system(cmd);

        memset(fname,0,MAXPATHLEN);
        sprintf(fname,"%s/%s.cu",rex_dir,rex_table[i].reg_ptr[j]->entry_name);
        omrpc_rename_func(fname,rex_table[i].reg_ptr[j]->entry_name);
        memset(cmd,0,1024);
        sprintf(cmd,"nvcc -arch=sm_20 -c %s_tmp.cu -o %s/%s.xmpgpu.o -I/%s/include",fname,rex_dir,rex_table[i].reg_ptr[j]->entry_name,prefix_dir);
        if(prt_log) printf("%s\n",cmd);
        system(cmd);

        sprintf(ctmp," %s/%s.o %s/%s.xmpgpu.o",rex_dir,rex_table[i].reg_ptr[j]->entry_name,rex_dir,rex_table[i].reg_ptr[j]->entry_name);
      }else{
        sprintf(ctmp," %s/%s.o",rex_dir,rex_table[i].reg_ptr[j]->entry_name);
      }
      strcat(buf, ctmp);
      if(prt_log) printf("    %s is included\n",rex_table[i].reg_ptr[j]->entry_name);
    }

    // pass all required libraries
    memset(ctmp,0,sizeof(ctmp));
#ifndef MYX
    sprintf(ctmp," %s/lib/libomrpc_stub.a %s/lib/libomrpc_io.a %s/lib/libxmp.a %s/lib/libymlworker.a %s/lib/libexpat.a -lm -ldl -fopenmp",prefix_dir,prefix_dir,prefix_dir,prefix_dir,prefix_dir);
#else
    sprintf(ctmp," %s/lib/libomrpc_stub.a %s/lib/libomrpc_io.a %s/lib/libxmp.a %s/lib/libymlworker.a %s/lib/libexpat.a %s -lcProtMpiSplitComm %s -lpnmpi -lm -ldl -fopenmp",prefix_dir,prefix_dir,prefix_dir,prefix_dir,prefix_dir,MUST_MODULE_DIR,MUST_LIB_DIR);
#endif
    strcat(buf, ctmp);
    for(j=0;j<rex_table[i].n_lib;j++){
      strcat(buf, " ");
      strcat(buf, rex_table[i].lib[j]);
    }
    printf("EXEC:%s\n",buf);
    system(buf);

    if(prt_log) printf("\n");
  }

  omprc_print_reg_table();
  
  return 0;
}  /* main */

int  omrpc_gets(char **ptr, char *ptr2, const int n)
{
  int i;
  for(i=0;i<n;i++){
    ptr2[i]=(*ptr)[i];
    if(ptr2[i]=='\n') break;
  }
  if(i<n && (*ptr)[i+1]!=0){
    ptr2[i+1]=0;
    *ptr=&((*ptr)[i+1]);
    return 1;
  }else{
    return 0;
  }
}  /* omrpc_gets */


void omrpc_print_idl_header(FILE *fp, int id)
{
  int  i;

  fprintf(fp,"Module %s;\n\n",rex_table[id].name);
  fprintf(fp,"Globals {\n\
#include <worker.h>\n\
#include<limits.h>\n\
#include\"xmp_header.hh\"\n\
#include\"myx_worker_wrapper.h\"\n\
MPI_Comm REX_COMM_WORLD=MPI_COMM_NULL;\n\
struct droperation\n\
{\n\
    char*          mHost;\n\
    unsigned short mPort;\n\
    char*          mUri;\n\
    uint8          mIsPacked;\n\
};\n\
typedef struct droperation droperation_t;\n\
struct worker\n\
{\n\
    drclient_t mClient;\n\
    char mInitialWorkingDir[PATH_MAX];\n\
    char mWorkingDir[PATH_MAX];\n\
    char* mWorkDescription;\n\
    uint32 mWorkDescriptionSize;\n\
    char* mBinaryName;\n\
\n\
    char** mParameters;\n\
    uint32 mParameterCurrent;\n\
    uint32 mParameterCount;\n\
\n\
    droperation_t* mImports;\n\
    uint32 mImportCurrent;\n\
    uint32 mImportCount;\n\
\n\
    droperation_t mExport;\n\
    droperation_t mAdmin;\n\
    uint8 mXmlErrorDetected;\n\
};\n\
typedef struct worker* worker_t;\n\
extern int worker_init(worker_t* context, char* data, uint32 dataSize, drclient_t client);\n\
extern int worker_import_resources(worker_t context);\n\
extern int worker_export_resouces(worker_t context);\n\
");
  for(i=0;i<rex_table[id].n_entries;i++){
    fprintf(fp,"extern void main_%s(int argc,char **argv);\n",rex_table[id].reg_ptr[i]->entry_name);
  }
  fprintf(fp,"}\n");

} /* omrpc_print_idl_header */

void omrpc_print_idl_def(FILE *fp, char *fname)
{
  char fin_name[MAXPATHLEN];
  char buf[MAX_BUF];
  FILE *fp_in;

  sprintf(fin_name, "%s/%s.idl",rex_dir,fname);
  fp_in = fopen(fin_name,"r");
  if(fp_in==NULL){
    omrpc_error_exit("cannot open file %s",fin_name);
  }
  
  // skip
  while(fgets(buf, MAX_BUF, fp_in)){
    if(!strncmp(buf,"Define",6)) break;
  }

  if(strncmp(buf,"Define",6)){
    fprintf(stderr,"WARNING : FILE %s doesn't include Definition\n",fin_name);
    fprintf(stderr,"WARNING : FILE %s is skipped\n",fin_name);
    fclose(fp_in);
    return;
  }
  
  // print definition 
  fprintf(fp,"%s",buf);
  while(fgets(buf,MAX_BUF,fp_in)){
    fprintf(fp,"%s",buf);
  }

  fclose(fp_in);
} /* omrpc_print_idl_def */

/* 
 * marge all *.idl files for entries in rex_table[id]
 */
void omrpc_marge_idl(int id)
{
  int  i, n;
  char fout_name[MAXPATHLEN];
  FILE *fp;  

  n = rex_table[id].n_entries;
  sprintf(fout_name,"%s/%s.idl",rex_dir,rex_table[id].name); 

  fp = fopen(fout_name, "w");
  if(fp==NULL){
    omrpc_error_exit("cannot open file %s",fout_name);
  }

  omrpc_print_idl_header(fp, id);

  for(i=0; i<n; i++){
    omrpc_print_idl_def(fp, rex_table[id].reg_ptr[i]->entry_name);
  }  
  fclose(fp);
} /* omrpc_marge_idl */

void omrpc_put_reg(char **ptr, char *fin_name)
{
  char buf[MAX_BUF];
  char entry_name[MAXPATHLEN];
  int  i, j, nprocs, type, narg;

  omrpc_gets(ptr,buf,MAX_BUF);
  type=atoi(buf);
  if(type!=2) return;

  omrpc_gets(ptr,buf,MAX_BUF); // skip
  omrpc_gets(ptr,buf,MAX_BUF); // skip

  omrpc_gets(ptr,entry_name,MAXPATHLEN); 
  entry_name[strlen(entry_name)-1]=0;

  omrpc_gets(ptr,buf,MAX_BUF);
  narg = atoi(buf);

  // skip
  for(i=0; i<narg; i++){
    omrpc_gets(ptr,buf,MAX_BUF);
  }

  omrpc_gets(ptr,buf,MAX_BUF);
  nprocs = atoi(buf);

  for(i=0; i<n_entries; i++){
    if(strcmp(entry_name, reg_table[i].entry_name)==0 && nprocs==reg_table[i].nprocs) break;
  }

  if(i>=n_entries){
    sprintf(reg_table[n_entries].entry_name,"%s",entry_name);
    sprintf(reg_table[n_entries].module_name,"%s_%d",fin_name,nprocs);
    reg_table[n_entries].nprocs=nprocs;  
    for(j=0; j<n_rexes; j++){
      if(strcmp(reg_table[n_entries].module_name, rex_table[j].name) == 0){
        reg_table[i].rex_id = j;         
        break;
      }
    }
    if(j>=n_rexes){
      sprintf(rex_table[n_rexes].name,"%s",reg_table[n_entries].module_name);
      sprintf(rex_table[n_rexes].rex_path,"%s/%s",rex_dir,reg_table[n_entries].module_name);
      rex_table[n_rexes].n_entries=1;
      rex_table[n_rexes].reg_ptr[rex_table[n_rexes].n_entries-1]=&(reg_table[n_entries]);
      n_rexes++;
    }else{
      rex_table[j].reg_ptr[rex_table[j].n_entries++]=&(reg_table[n_entries]);
    }
    n_entries++;
  }
} /* omrpc_put_reg */

void omprc_print_reg_table()
{
  int i;
  for(i=0; i<n_entries; i++){
    printf("%s %s %s/%s.rex\n",reg_table[i].module_name,reg_table[i].entry_name,rex_dir,reg_table[i].module_name);
  } 
}  /* omprc_print_reg_table */

void omrpc_add_libtxt(char *txt, rex_t *rex)
{
  int i,j;

  for(i=0;i<MAX_BUF &&(txt[i]==32);i++);

  if(rex->n_libtxt==0){
    if(prt_log) printf("libs=%s,",&(txt[i]));
    strcpy(rex->libtxt[rex->n_libtxt++],&(txt[i]));
  }else{
    for(j=0;j<rex->n_libtxt;j++){
      if(!strcmp(rex->libtxt[j],&(txt[i]))) break;
    }
    if(j>=rex->n_libtxt){ 
      if(prt_log) printf("libs=%s,",&(txt[i]));
      strcpy(rex->libtxt[rex->n_libtxt++],&(txt[i]));
    }
  }
}  /* omrpc_add_libtxt */

void omrpc_get_lang(char *fname, char *lang)
{
  int    i,j,k;
  char   buf[MAX_BUF];
  FILE   *fp;

  fp=fopen(fname,"r");
  if(fp==NULL){
    omrpc_error_exit("%s cannot open file %s",__func__,fname);
  }
  while(fgets(buf, MAX_BUF, fp)){
    if(strstr(buf,"<impl")) break;
  }
  fclose(fp);
  if(!strstr(buf,"lang")){
    omrpc_error_exit("%s no language is specified in %s",__func__,fname);
  }

  for(i=0,j=0;i<MAX_BUF-5;i++){
    if(!strncmp(&(buf[i]),"lang",4)){
      j=i+4; break;
    }
  }
  for(i=j;i<MAX_BUF;i++){
    if(buf[i]=='='){
      j=i+1; break;
    }
  }

  for(i=j;i<MAX_BUF;i++){
    if(buf[i]=='"'){
      j=i+1; break;
    }
  }

  for(i=j,k=0;i<MAX_BUF;i++){
    if(buf[i]=='"'){
      lang[k]=0;
      break;
    }
    if(buf[i]==' ' && k!=0){
      lang[k]=0;
      k=0;
    }else if(buf[i]==' ' && k==0){
      continue;
    }
    lang[k++]=buf[i];
  }
} /* omrpc_get_lang */

void omrpc_get_libtxt(char *fname, rex_t *rex)
{
  int  i, j, k;
  char buf[MAX_BUF];
  char txt[MAX_BUF];
  char nolib[MAX_BUF];
  FILE *fp;

  sprintf(nolib,"libs=\"\"");

  fp=fopen(fname,"r");
  if(fp==NULL){
    omrpc_error_exit("%s cannot open file %s",__func__,fname);
  }

  while(fgets(buf, MAX_BUF, fp)){
    if(strstr(buf,"<impl")) break;
  }
  fclose(fp);
  if(!strstr(buf,"libs")) return;
  if(strstr(buf,nolib))   return;

  for(i=0,j=0;i<MAX_BUF-5;i++){
    if(!strncmp(&(buf[i]),"libs",4)){ 
      j=i+4; break;
    }
  }

  for(i=j;i<MAX_BUF;i++){
    if(buf[i]=='='){ 
      j=i+1; break;
    }
  }

  for(i=j;i<MAX_BUF;i++){
    if(buf[i]=='"'){ 
      j=i+1; break;
    }
  }

  for(i=j,k=0;i<MAX_BUF;i++){
    if(buf[i]=='"'){ 
      txt[k]=0;
      omrpc_add_libtxt(txt,rex);
      break;
    }
    if(buf[i]==' ' && k!=0){
      txt[k]=0;
      omrpc_add_libtxt(txt,rex);
      k=0;      
    }else if(buf[i]==' ' && k==0){
      continue;
    }
    txt[k++]=buf[i];
  }
}  /* omrpc_get_libtxt */

void omrpc_add_lib(char *txt, rex_t*rex)
{
  int j;
  if(rex->n_lib==0){
    if(prt_log) printf("    %s is added\n",txt);
    strcpy(rex->lib[rex->n_lib++],txt);
  }else{
    for(j=0;j<rex->n_lib;j++){
      if(!strcmp(rex->lib[j],txt)) break;
    }
    if(j>=rex->n_lib){ 
      if(prt_log) printf("    %s is added\n",txt);
      strcpy(rex->lib[rex->n_lib++],txt);
    }
  }
}  /* omrpc_add_lib */

void omrpc_get_lib(rex_t *rex)
{
  int  i, len;
  char fname[MAXPATHLEN];
  char buf[MAX_BUF];
  FILE *fp;

  if(prt_log) printf("\n");
  for(i=0;i<rex->n_libtxt;i++){
    memset(fname,0,sizeof(fname));
    sprintf(fname,"%s/%s.txt",lib_dir,rex->libtxt[i]);
    if(prt_log) printf("  Open %s\n",fname);
    fp=fopen(fname,"r");
    if(fp==NULL){
      omrpc_error_exit("cannot open file %s",fname);
    }

    fgets(buf, MAX_BUF, fp); // include
    fgets(buf, MAX_BUF, fp); // lib
    fclose(fp);

    len=strlen(buf);
    if(buf[len-1]=='\n') buf[len-1]=0;
    if(prt_log) printf("  Library %s is found\n",buf);
    omrpc_add_lib(buf, rex);
  }

}  /* omrpc_get_lib */

void omrpc_gsub(const char *in, char *out, const char *s1, const char *s2)
{
  int i, j;
  int len0, len1, len2;

  len0=strlen(in);
  len1=strlen(s1);
  len2=strlen(s2);

  for(i=0,j=0; (i<len0 && in[i]!='\n' && in[i]!=0 && j<MAXPATHLEN);){
    if(!strncmp(&(in[i]),s1,len1)){
      strcat(out,s2);
      j+=len2;
      i+=len1;
    }else{
      out[j++]=in[i++];
    }
    out[j]=0;
  }
}  /* omrpc_gsub */

void omrpc_error_exit(char *msg,...)
{
  va_list ap;
  va_start(ap,msg);
  fprintf(stderr,"ERROR: ");
  vfprintf(stderr,msg,ap);
  fprintf(stderr,"\n");
  va_end(ap);
  exit(1);
}  /* omrpc_error_exit */

void omrpc_set_dir()
{
  char tmp[MAXPATHLEN];

  sprintf(tmp,"%s\n",PREFIX_DIR);
  omrpc_gsub(tmp, prefix_dir, "$(DESTDIR)", "");

  memset(rex_dir,0,sizeof(rex_dir));
  memset(lib_dir,0,sizeof(lib_dir));
  memset(inc_dir,0,sizeof(inc_dir));
  memset(imp_dir,0,sizeof(imp_dir));
  sprintf(rex_dir,"%s/%s",prefix_dir,REX_DIR);
  sprintf(lib_dir,"%s/%s",prefix_dir,LIB_DIR);
  sprintf(inc_dir,"%s/%s",prefix_dir,INC_DIR);
  sprintf(imp_dir,"%s/%s",prefix_dir,IMP_DIR);

}  /* omrpc_set_dir */

void omrpc_rename_func(char *file_name, char *func_name)
{
  char   tmp[256],str1[1024],str2[1024],*p;
  FILE   *fp_in, *fp_out;

  p=strrchr(file_name,46);
  sprintf(tmp,"%s_tmp%s",file_name,p);

  fp_in =fopen(file_name,"r");
  fp_out=fopen(tmp,      "w");
  while(fgets(str1,1024,fp_in)){
    p=strstr(str1,"_XMP_STARPU_FUNC_");
    if(p){
      memset(str2,0,1024);
      strncat(str2,str1,p-str1+17);
      strcat(str2,func_name);
      strcat(str2,"_");
      strcat(str2,p+17);
      fputs(str2,fp_out);
    }else{
      fputs(str1,fp_out);
    }
  }

  fclose(fp_in);
  fclose(fp_out);
/*
  memset(str1,0,1024);
  sprintf(str1,"mv %s %s",tmp,file_name);
  system(str1);
*/
} /* omrpc_rename_func */

