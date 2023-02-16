 /*  $Id: omrpc_exec.c,v 1.2 2006-01-25 16:06:17 ynaka Exp $
	*  $Release: omnirpc-2.0.1 $
	*  $Copyright:
	*   OmniRPC Version 1.0
	*   Copyright (C) 2002-2004 HPCS Laboratory, University of Tsukuba.
	*   
	*   This software is free software; you can redistribute it and/or modify
	*   it under the terms of the GNU Lesser General Public License version
	*   2.1 published by the Free Software Foundation.
	*   
	*   Please check the Copyright and License information in the files named
	*   COPYRIGHT and LICENSE under the top  directory of the OmniRPC Grid PRC 
	*   System release kit.
	*   
	*   
	*   $
	*/
#include "omrpc_defs.h"
#include "omrpc_exec.h"
#include "omrpc_mon.h"
#include "myx_master_wrapper.h"

/*
 *  exec process operation
 */
char *omrpc_RSH_command = RSH_COMMAND;
char *omrpc_SSH_command = SSH_COMMAND;
char *omrpc_PBS_SUBMIT_command = PBS_SUBMIT_COMMAND;
char *omrpc_SGE_SUBMIT_command = SGE_SUBMIT_COMMAND;

static void dump_exec_args(char *args[]);
static long int omrpc_pbs_num_jobs = 0;
static long int omrpc_sge_num_jobs = 0;
/* return pid */
int omrpc_exec_by_fork(char *path,
                       char *host,
                       unsigned short port_num,
                       int globus_flag,
                       char *working_path)
{
    int ac;
    char *arg[20];
    char port_arg[10];
    char *prog;
    int pid;

    snprintf(port_arg,sizeof(port_arg),"%d",port_num);

    ac = 0;
    prog = path;
    arg[ac++] = path;
    if(omrpc_debug_flag) arg[ac++] = "-debug";
    if(globus_flag) arg[ac++] = "-globus";
    if(working_path != NULL && strlen(working_path) != 0){
        arg[ac++] = "-workpath";
        arg[ac++] = working_path;
    }
    if(host != NULL){
        arg[ac++] = "-host";
        arg[ac++] = host;
    }
    arg[ac++] = "-port";
    arg[ac++] = port_arg;
    arg[ac++] = NULL;

    if(omrpc_debug_flag) dump_exec_args(arg);

    if((pid = fork()) == 0){
        execvp(prog,arg);
        if(omrpc_debug_flag) perror("execvp failed:");
        exit(1);
    }

    return pid;
}

int omrpc_exec_by_rsh(char *exec_host,
                      char *user_name,
                      char *path,
                      char *host,
                      unsigned short port_num,
                      int mxio_flag,
                      int globus_flag,
                      char *job_sched_type,
                      char *reg_path,
                      char *working_path)
{
    int ac;
    char *arg[25];
    char port_arg[10];
    char mon_port_arg[10];
    char *prog;
    int pid;
    extern char *omrpc_my_hostname;

    if(host == NULL) host = omrpc_my_hostname;

    snprintf(port_arg,sizeof(port_arg),"%d",port_num);

    ac = 0;
    prog = omrpc_RSH_command;
    arg[ac++] = omrpc_RSH_command;
    arg[ac++] = exec_host;
    if(user_name != NULL){
      arg[ac++] = "-l";
      arg[ac++] = user_name;
    }
    arg[ac++] = "-n";
    arg[ac++] = path;
    if(omrpc_debug_flag) arg[ac++] = "-debug";
    if(globus_flag) arg[ac++] = "-globus";
    if(mxio_flag) arg[ac++] = "-mxio";
    if(job_sched_type != NULL){
        arg[ac++] = "-sched";
        arg[ac++] = job_sched_type;
    }
    if(reg_path != NULL){
        arg[ac++] = "-reg";
        arg[ac++] = reg_path;
    }
#ifdef USE_MONITOR
    if(omrpc_monitor_flag){
        snprintf(mon_port_arg, sizeof(mon_port_arg),
                 "%d",omrpc_monitor_port);

        arg[ac++] = "-mon";
        if(omrpc_monitor_target_host != NULL){
            arg[ac++] = omrpc_monitor_target_host;
        } else {
            arg[ac++] = omrpc_my_hostname;
        }

        arg[ac++] = "-mon-port";
        arg[ac++] = mon_port_arg;
    }
#endif
    if(working_path != NULL && strlen(working_path) != 0){
        arg[ac++] = "-workpath";
        arg[ac++] = working_path;
    }
    arg[ac++] = "-host";
    arg[ac++] = host;
    arg[ac++] = "-port";
    arg[ac++] = port_arg;
    arg[ac++] = NULL;

    if(omrpc_debug_flag) dump_exec_args(arg);

    if((pid = fork()) == 0){
        execvp(prog,arg);
        if(omrpc_debug_flag) perror("execvp failed:");
        exit(1);
    }

    return pid;
}

int omrpc_exec_by_ssh(char *exec_host,
                      char *user_name,
                      char *path,
                      char *host,
                      unsigned short port_num,
                      int mxio_flag,
                      int globus_flag,
                      char *job_sched_type,
                      char *reg_path,
                      char *working_path)
{
    int ac;
    char *arg[40];
    char port_arg[10];
    char port_forward_arg[50];
    char mon_port_arg[10];
    char *prog;
    int pid;
    extern char *omrpc_my_hostname;

    snprintf(port_arg,sizeof(port_arg),"%d",port_num);
    snprintf(port_forward_arg,sizeof(port_forward_arg),
             "%d:localhost:%d",port_num,port_num);

    ac = 0;
    prog = omrpc_SSH_command;
    arg[ac++] = omrpc_SSH_command;
    if(user_name != NULL){
      arg[ac++] = "-l";
      arg[ac++] = user_name;
    }
    arg[ac++] = "-n";
    arg[ac++] = "-R";
    arg[ac++] = port_forward_arg;
    arg[ac++] = "-q";

    arg[ac++] = exec_host;
    arg[ac++] = path;
    if(omrpc_debug_flag) arg[ac++] = "-debug";
    if(globus_flag) arg[ac++] = "-globus";
    if(mxio_flag) arg[ac++] = "-mxio";
    if(job_sched_type != NULL){
        arg[ac++] = "-sched";
        arg[ac++] = job_sched_type;
    }
    if(reg_path != NULL){
        arg[ac++] = "-reg";
        arg[ac++] = reg_path;
    }
#ifdef USE_MONITOR
    if(omrpc_monitor_flag){
        snprintf(mon_port_arg, sizeof(mon_port_arg),
                 "%d",omrpc_monitor_port);

        arg[ac++] = "-mon";
        if(omrpc_monitor_target_host != NULL){
            arg[ac++] = omrpc_monitor_target_host;
        } else {
            arg[ac++] = omrpc_my_hostname;
        }

        arg[ac++] = "-mon-port";
        arg[ac++] = mon_port_arg;
    }
#endif
    if(working_path != NULL && strlen(working_path) != 0){
        arg[ac++] = "-workpath";
        arg[ac++] = working_path;
    }
    arg[ac++] = "-host";
    arg[ac++] = host;
    arg[ac++] = "-port";
    arg[ac++] = port_arg;
    arg[ac++] = "-port-forwarding";    /* must be localhost 127.0.0.1 */
    arg[ac++] = NULL;

    if(omrpc_debug_flag) dump_exec_args(arg);

    if((pid = fork()) == 0){
        execvp(prog,arg);
        if(omrpc_debug_flag) perror("execvp failed:");
        exit(1);
    }

    return pid;
}

int omrpc_exec_by_pbs(char *path,
                      char *host,
                      unsigned short port_num,
                      int mxio_flag,
                      int globus_flag,
                      char *job_sched_type,
                      char *working_path)
{
    int ac;
    char *arg[20];
    char port_arg[10];
    char *script_name;   /* job script */
    char *prog;
    int pid;
    FILE *script_fp;

    extern char *omrpc_my_hostname;

    if(host == NULL) host = omrpc_my_hostname;

    snprintf(port_arg, sizeof(port_arg), "%d",port_num);

    /* set job script */
    script_name = omrpc_tempnam("pbs",omrpc_pbs_num_jobs++);
    if((script_fp = fopen(script_name, "w+")) == NULL){
        perror("script file cannot create");
    }
    fprintf(script_fp, "#!/bin/sh\n");
    fprintf(script_fp, "%s ", path);
    if(omrpc_debug_flag)
        fprintf(script_fp, "-debug ");
    if(globus_flag)
        fprintf(script_fp, "-globus ");
    if(mxio_flag)
        fprintf(script_fp, "-mxio ");
    if(job_sched_type != NULL)
        fprintf(script_fp, "-sched %s ", job_sched_type);
    if(working_path != NULL && strlen(working_path) != 0){
        fprintf(script_fp, "-workpath %s ", working_path);
    }

    fprintf(script_fp, "-host %s -port %s\n", host, port_arg);
    if(fclose(script_fp) == EOF) perror("cannot close script file");

    /* set pbs command */
    ac = 0;
    prog = omrpc_PBS_SUBMIT_command;
    arg[ac++] = omrpc_PBS_SUBMIT_command;
    arg[ac++] = "-N";
    arg[ac++] = "omrpc_call";
    if(! omrpc_debug_flag){
        arg[ac++] = "-o";
        arg[ac++] = "/dev/null";
        arg[ac++] = "-j";
        arg[ac++] = "oe";
        arg[ac++] = "-m";
        arg[ac++] = "n";
    }
    arg[ac++] = script_name;
    arg[ac++] = NULL;

    if(omrpc_debug_flag) dump_exec_args(arg);

    if((pid = fork()) == 0){
        execvp(prog,arg);
        if(omrpc_debug_flag) perror("execvp failed:");
        if(!omrpc_debug_flag)
            if(unlink(script_name) < 0) perror("unlink failed:");
        free(script_name);
        exit(1);
    }
    free(script_name);

    return pid;
}

int omrpc_exec_by_sge(char *path,
                      char *host,
                      unsigned short port_num,
                      int mxio_flag,
                      int globus_flag,
                      char *job_sched_type,
                      char *working_path)
{
    int ac;
    char *arg[20];
    char port_arg[10];
    char *script_name;   /* job script */
    char *prog;
    int pid;
    FILE *script_fp;

    extern char *omrpc_my_hostname;

    if(host == NULL) host = omrpc_my_hostname;

    snprintf(port_arg,sizeof(port_arg),"%d",port_num);

    /* set job script */
    script_name = omrpc_tempnam("sge", omrpc_sge_num_jobs++);
    if((script_fp = fopen(script_name, "w+")) == NULL){
        perror("script file cannot create");
    }
    fprintf(script_fp, "#!/bin/sh\n");
    fprintf(script_fp, "%s ", path);
    if(omrpc_debug_flag)
        fprintf(script_fp, "-debug ");
    if(globus_flag)
        fprintf(script_fp, "-globus ");
    if(mxio_flag)
        fprintf(script_fp, "-mxio ");
    if(job_sched_type != NULL)
        fprintf(script_fp, "-sched %s ", job_sched_type);
    if(working_path != NULL && strlen(working_path) != 0){
        fprintf(script_fp, "-workpath %s ", working_path);
    }
    fprintf(script_fp, "-host %s -port %s\n", host, port_arg);

    if(fclose(script_fp) == EOF) perror("cannot close script file");

    /* set pbs command */
    ac = 0;
    prog = omrpc_SGE_SUBMIT_command;
    arg[ac++] = omrpc_SGE_SUBMIT_command;
    arg[ac++] = "-N";
    arg[ac++] = "omrpc_call";
    if(! omrpc_debug_flag){
        arg[ac++] = "-o";
        arg[ac++] = "/dev/null";
        arg[ac++] = "-j";
        arg[ac++] = "oe";
        arg[ac++] = "-m";
        arg[ac++] = "n";
    }
    arg[ac++] = script_name;
    arg[ac++] = NULL;

    if(omrpc_debug_flag) dump_exec_args(arg);

    if((pid = fork()) == 0){
        execvp(prog,arg);
        if(omrpc_debug_flag) perror("execvp failed:");
        if(!omrpc_debug_flag)
            if(unlink(script_name) < 0) perror("unlink failed:");
        free(script_name);
        exit(1);
    }
    free(script_name);

    return pid;
}

int omrpc_exec_worker_by_ssh(char *exec_host,
                             char *user_name,
                             char *path,
                             char *host,
                             unsigned short port_num,
                             int mxio_flag,
                             int globus_flag,
                             char *job_sched_type,
                             char *working_path)
{
    int ac;
    char *arg[40];
    char port_arg[10];
    char mon_port_arg[10];
    char *prog;
    int pid;
    extern char *omrpc_my_hostname;

    if(host == NULL) host = omrpc_my_hostname;
    snprintf(port_arg,sizeof(port_arg),"%d",port_num);

    ac = 0;
    prog = omrpc_SSH_command;
    arg[ac++] = omrpc_SSH_command;
    if(user_name != NULL){
      arg[ac++] = "-l";
      arg[ac++] = user_name;
    }

    arg[ac++] = exec_host;
    arg[ac++] = path;
    if(omrpc_debug_flag) arg[ac++] = "-debug";
    if(globus_flag) arg[ac++] = "-globus";
    if(mxio_flag) arg[ac++] = "-mxio";
    if(job_sched_type != NULL){
        arg[ac++] = "-sched";
        arg[ac++] = job_sched_type;
    }
#ifdef USE_MONITOR
    if(omrpc_monitor_flag){
        snprintf(mon_port_arg,sizeof(mon_port_arg),"%d",omrpc_monitor_port);
        arg[ac++] = "-mon";
        if(omrpc_monitor_target_host != NULL){
            arg[ac++] = omrpc_monitor_target_host;
        } else {
            arg[ac++] = omrpc_my_hostname;
        }
        arg[ac++] = "-mon-port";
        arg[ac++] = mon_port_arg;
    }
#endif
    if(working_path != NULL && strlen(working_path) != 0){
        arg[ac++] = "-workpath";
        arg[ac++] = working_path;
    }
    arg[ac++] = "-host";
    arg[ac++] = host;
    arg[ac++] = "-port";
    arg[ac++] = port_arg;
    arg[ac++] = NULL;

    if(omrpc_debug_flag) dump_exec_args(arg);

    if((pid = fork()) == 0){
        execvp(prog,arg);
        if(omrpc_debug_flag) perror("execvp failed:");
        exit(1);
    }

    return pid;
}

#ifdef USE_MPI
int omrpc_exec_by_mpi(char *path,
                      char *host,
                      unsigned short port_num,
                      char *working_path,
                      int  nprocs,
                      char *schd, // scheduling policy. "MPI" is MPI only, "mpi" is mpi and omnirpc
                      MPI_Comm *comm){
    int      i,pid=0,ac;
    char     *arg[40];
    char     port_arg[16];

    snprintf(port_arg,sizeof(port_arg),"%u",port_num);

    ac = 0;
    if(omrpc_debug_flag) arg[ac++] = "-debug";
    if(working_path != NULL && strlen(working_path) != 0){
        arg[ac++] = "-workpath";
        arg[ac++] = working_path;
    }
    if(host != NULL){
        arg[ac++] = "-host";
        arg[ac++] = host;
    }
    arg[ac++] = "-port";
    arg[ac++] = port_arg;
    arg[ac++] = "-sched";
    arg[ac++] = schd;
    arg[ac++] = NULL;

    if(omrpc_debug_flag) dump_exec_args(arg);

    MPI_Comm_spawn(path,arg,nprocs,MPI_INFO_NULL,0,MPI_COMM_SELF,comm,MPI_ERRCODES_IGNORE);
    return pid; 
}
#endif /* USE_MPI */

static void dump_exec_args(char *args[])
{
    int i;
    omrpc_prf("dump exec args:\n");
    for(i = 0; args[i] != NULL; i++)
        omrpc_prf("exec args[%d]: %s\n",i, args[i]);
}


