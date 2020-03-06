/* 
 * $Id: omrpc-register.c,v 1.3 2006-01-31 17:36:55 ynaka Exp $
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
 *  
 *  $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "omrpc_defs.h"
#include "omrpc_rpc.h"
#include "ninf_stub_info.h"

/*
 *  OmniRPC register program
 */
#define MAX_ENTRY 1024

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif /* !MAXPATHLEN */

#define MAX_LINE_BUF 65536

char *program;
char pwd[MAXPATHLEN + 1];
char registry_file[MAXPATHLEN + 1];

struct {
    char *module_name;
    char *entry_name;
    char *stub_path;
} reg_table[MAX_ENTRY];

int n_entry = 0;

extern int omrpc_debug_flag;

#define STR_EQ(s1,s2) (strcmp(s1,s2) == 0)

/* protypes */
void usage(void);
void read_registry_file(void);
void write_registry_file(void);
void print_registry(void);
void register_stub_program(char *stub_prog);
void remove_stub_program(char *stub_prog);

void usage()
{
    fprintf(stderr, "%s [-path registry_path] [-clear|-show | [[-register | -remove] filenames]]]\n",program);
}

int
main(int argc, char *argv[])
{
    char *path = NULL;
    char cwd[MAXPATHLEN + 1];
    struct stat sbuf;
    int r;

    omrpc_io_init();
    omrpc_init_client();

    if (getcwd(cwd, MAXPATHLEN) == NULL) {
	perror("getcwd:");
	return 1;
    } else {
	strcpy(pwd, cwd);
	strcat(pwd, "/");
    }

    program = *argv;
    argc--;
    argv++;

    if(argc > 0 && STR_EQ(*argv,"-debug")){
	omrpc_debug_flag++;
	argc--;
	argv++;
    }

    if(argc > 0 && STR_EQ(*argv,"-path")){
	argc--;
	argv++;
	if(argc > 0){
	    strcpy(registry_file,*argv);
	    argc--;
	    argv++;
	} else goto bad_arg;
    } else {
	if((path = (char *)getenv("OMRPC_REG_PATH")) == NULL){
	    if((path = (char *)getenv("HOME")) == NULL){
		fprintf(stderr,"cannot get 'HOME',"
			" using current directory for registry.\n");
		strcpy(registry_file,".");
	    } else {
		strcpy(registry_file,path);
		strcat(registry_file,"/");
		strcat(registry_file,HOME_REG_DIR);
		r = stat(registry_file,&sbuf);
		if(r < 0){ /* file is not found */
		    r = mkdir(registry_file,0700);
		    if(r < 0){
			fprintf(stderr,
				"cannot create registry directory '%s'\n",
				registry_file);
			exit(1);
		    }
		} else {
		    if(!S_ISDIR(sbuf.st_mode)){
			fprintf(stderr,
				"registry directory '%s' is not directory\n",
				registry_file);
			exit(1);
		    }
		}
	    }
	} else {
	    strcpy(registry_file,path);
	}
    }

    strcat(registry_file,"/");
    strcat(registry_file,REGISTRY_FILE);

    if (omrpc_debug_flag) {
	fprintf(stderr, "registry file='%s'\n",registry_file);
    }
    
    read_registry_file();

    if(argc > 0 && STR_EQ(*argv,"-show")){
	print_registry();
	return 0;
    }

    if(argc > 0 && STR_EQ(*argv,"-help")){
	usage();
	return 0;
    }

    if(argc > 0 && STR_EQ(*argv,"-clear")){
	n_entry = 0;
	argc--;
	argv++;
    }

    while(argc > 0){
	if(STR_EQ(*argv,"-register")){
	    argc--;
	    argv++;
	    while(argc > 0 && *argv[0] != '-'){
		register_stub_program(*argv);
		argc--;
		argv++;
	    }
	    continue;
	}
	if(STR_EQ(*argv,"-remove")){
	    argc--;
	    argv++;
	    while(argc > 0 && *argv[0] != '-'){
		remove_stub_program(*argv);
		argc--;
		argv++;
	    }
	    continue;
	}
	goto bad_arg;
    }

    write_registry_file();

    omrpc_io_finalize();
    omrpc_finalize_client();

    return 0;

bad_arg:
    fprintf(stderr,"bad arg\n");
    usage();
    return 1;
}

void read_registry_file()
{
    FILE *fp;
    char path[MAXPATHLEN + 1];
    char linebuf[MAX_LINE_BUF];
    char module_name[MAXPATHLEN + 1],entry_name[MAXPATHLEN + 1];
    int n;

    if ((fp = fopen(registry_file,"r")) == NULL) return;

    if (omrpc_debug_flag) {
	fprintf(stderr, "reading regsitry file ...\n");
    }

    while(fgets(linebuf,MAX_LINE_BUF,fp) != NULL){
	if(linebuf[0] == '#') continue;
	n = sscanf(linebuf,"%s %s %s",module_name,entry_name,path);
        if(n != 3){
	    fprintf(stderr,"bad registory file\n");
	    exit(0);
	}

	if (omrpc_debug_flag) {
	    fprintf(stderr, "%s %s %s\n",entry_name,module_name,path);
	}

        reg_table[n_entry].entry_name = strdup(entry_name);
        reg_table[n_entry].module_name = strdup(module_name);
        reg_table[n_entry].stub_path = strdup(path);
	n_entry++;
    }
    fclose(fp);
}

void write_registry_file()
{
    FILE *fp;
    int i;
    
    if((fp = fopen(registry_file,"w+")) == NULL){
	fprintf(stderr,"cannot open registry file,'%s'\n",
		registry_file);
	exit(1);
    }
    fprintf(fp,"# this file is generated by %s, don't edit\n", program);
    fprintf(fp,"# module_name entry_name stub_path\n");
    for(i = 0; i < n_entry; i++){
	if(reg_table[i].entry_name == NULL) continue;
	fprintf(fp,"%s %s %s\n",
		reg_table[i].module_name,reg_table[i].entry_name,
		reg_table[i].stub_path);
    }
    fclose(fp);
}

void print_registry()
{
    int i;

    fprintf(stdout, " The number of stubs = %d\n",n_entry);
    for(i = 0; i < n_entry; i++){
	if(reg_table[i].entry_name == NULL) continue;
	fprintf(stdout, "%s\t%s\t%s\n",reg_table[i].entry_name,
	       reg_table[i].module_name,reg_table[i].stub_path);
    }
}

void register_stub_program(char *stub_prog)
{
    char name[MAXPATHLEN + 1];
    int i,index;
    omrpc_rpc_t *rp;
    NINF_STUB_INFO *sp;

    if (omrpc_debug_flag) {
	fprintf(stderr, "register stub '%s' ...\n",stub_prog);
    }
    if(*stub_prog == '/') name[0] = '\0';
    else strcpy(name,pwd);
    strcat(name,stub_prog);

    if (omrpc_debug_flag) {
	fprintf(stderr, "exec stub '%s' ...\n",name);
    }
    rp = omrpc_exec_on_host(NULL,name);
    if(rp == NULL){
	fprintf(stderr,"error: bad stub program '%s'\n",name);
	exit(1);
    }

    for(index = 0; index < rp->n_entry; index++){
	omrpc_req_stub_info_local_by_index(rp->hp,index);
	omrpc_recv_stub_info_local(rp->hp,&sp);

	for(i = 0; i < n_entry; i++){
	    if(reg_table[i].entry_name == NULL) continue;
	    if(STR_EQ(reg_table[i].entry_name,sp->entry_name) &&
	       STR_EQ(reg_table[i].module_name,sp->module_name)){
		reg_table[i].stub_path = strdup(name);
		fprintf(stdout, "%s:%s was updated. (stub='%s')\n",
			sp->entry_name,sp->module_name,name);
		break;
	    }
	}
	if(i == n_entry){
	    reg_table[n_entry].stub_path = strdup(name);
	    reg_table[n_entry].entry_name = strdup(sp->entry_name);
	    reg_table[n_entry].module_name = strdup(sp->module_name);
	    n_entry++;
	    fprintf(stdout, "%s:%s was registered.(stub='%s')\n",
		    sp->entry_name,sp->module_name,name);
	}
    }

    omrpc_exec_terminate(rp);
}

void remove_stub_program(char *stub_prog)
{
    char name[MAXPATHLEN + 1];
    int i;
    if(*stub_prog == '/') name[0] = '\0';
    else strcpy(name,pwd);
    strcat(name,stub_prog);
    for(i = 0; i < n_entry; i++){
	if(reg_table[i].entry_name == NULL) continue;
	if(STR_EQ(reg_table[i].stub_path,name)){
	    fprintf(stdout, "%s:%s was removed.(stub='%s')\n",
		    reg_table[i].entry_name,reg_table[i].module_name,name);
	    reg_table[i].entry_name = NULL;
	    return;
	}
    }
    fprintf(stdout, "stub '%s' for remove was not found.\n",name);
}

