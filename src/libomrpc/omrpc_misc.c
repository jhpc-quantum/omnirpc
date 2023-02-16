static char rcsid[] = "$Id: omrpc_misc.c,v 1.1.1.1 2004-11-03 21:01:17 yoshihiro Exp $";
/*
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
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "omrpc_defs.h"

int omrpc_debug_flag;
int omrpc_use_globus;
char *omrpc_my_prog_name;


void omrpc_get_default_path(char *buf,char *file)
{
    char *path;

    if((path = (char *)getenv("OMRPC_REG_PATH")) == NULL){
        if((path = (char *)getenv("HOME")) == NULL){
            omrpc_prf("cannot get 'HOME', using current directory"
                      "for getting machine file.\n");
            strcpy(buf,".");
        } else {
            strcpy(buf,path);
            strcat(buf,"/");
            strcat(buf,HOME_REG_DIR);
        }
    } else strcpy(buf,path);
    strcat(buf,"/");
    strcat(buf,file);
}

/* memory allocator */
void *omrpc_malloc(size_t size)
{
    void *p;
    p = (void *)malloc(size);
    if(p == NULL) {
        omrpc_fatal("cannot allocate memory");
    }
    bzero(p,size);
    return p;
}

void omrpc_free(void *p)
{
    free(p);
}

void omrpc_fatal(const char *msg,...)
{
  extern char *omrpc_my_hostname;
    va_list ap;
    va_start(ap,msg);
    fprintf(stderr,"OMRPC_FATAL(%s:%s): ",omrpc_my_hostname, omrpc_my_prog_name);
    vfprintf(stderr,msg,ap);
    fprintf(stderr,"\n");
    va_end(ap);
    exit(1);
}

void omrpc_error(const char *msg,...)
{
    va_list ap;
    va_start(ap,msg);
    fprintf(stderr,"OmniRpc Error(%s): ",omrpc_my_prog_name);
    vfprintf(stderr,msg,ap);
    fprintf(stderr,"\n");
    va_end(ap);
    exit(1);
}

void omrpc_prf(const char *msg,...)
{
    extern char *omrpc_my_hostname;
    int l;
    char s[12];

    va_list ap;
    va_start(ap,msg);
    l = strlen(omrpc_my_prog_name);
    if(l < 10) strcpy(s,omrpc_my_prog_name);
    else {
        strcpy(s,"**");
        strncpy(s+2,omrpc_my_prog_name+l-8,9);
    }
    fprintf(stdout,"[%s:%s:%x] ",omrpc_my_hostname, s, getpid());
    vfprintf(stdout,msg,ap);
    fflush(stdout);
    va_end(ap);
}

char* omrpc_tempnam(char *base, int id)
{
    char *tmp;
    tmp = omrpc_malloc(sizeof(char)*128);
    snprintf(tmp, 128, "%s/%s%s.u%d.p%d.%d",
             OMRPC_WORKDIR, "omrpc", base, (int)getuid(), (int)getpid(), id);

    return tmp;
}
