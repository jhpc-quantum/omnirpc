/*
 * $Id: omrpc_exec.h,v 1.1.1.1 2004-11-03 21:01:14 yoshihiro Exp $
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
#ifndef _OMRPC_EXEC_H_
#define _OMRPC_EXEC_H_

#define RSH_COMMAND "rsh"       /* default */
#define SSH_COMMAND "ssh"       /* default */
#define PBS_SUBMIT_COMMAND "qsub"       /* default */
#define SGE_SUBMIT_COMMAND "qsub"       /* default */

extern char *omrpc_RSH_command;
extern char *omrpc_SSH_command;
extern char *omrpc_PBS_SUBMIT_command;
extern char *omrpc_SGE_SUBMIT_command;

int omrpc_exec_by_fork(char *path,char *host,unsigned short port_num,
                       int globus_flag,char *working_path);
int omrpc_exec_by_rsh(char *exec_host,char *user_name,
                      char *path,char *host,
                      unsigned short port_num,int mx_flag,
                      int globus_flag, char *job_sched_type,
                      char *reg_path, char *working_path);
int omrpc_exec_by_ssh(char *exec_host,char *user_name,
                      char *path,char *host,
                      unsigned short port_num,int mx_flag,
                      int globus_flag,char *job_sched_type,
                      char *reg_path,char *working_path);
int omrpc_exec_by_pbs(char *path,char *host,
                      unsigned short port_num,int mx_flag,
                      int globus_flag,char *job_sched_type,char *working_path);
int omrpc_exec_by_sge(char *path,char *host,
                      unsigned short port_num,int mx_flag,
                      int globus_flag,char *job_sched_type,char *working_path);
int omrpc_exec_worker_by_ssh(char *exec_host,char *user_name,
                             char *path,char *host,
                             unsigned short port_num,int mx_flag,
                             int globus_flag,char *job_sched_type,
                             char *working_path);
#ifdef USE_MPI
int omrpc_exec_by_mpi(char *path,char *host,unsigned short port_num, 
                      char *working_path, int nprocs, char *schd,
		      MPI_Comm *intercomm);
#endif /* USE_MPI */


#endif /*_OMRPC_EXEC_H_*/

