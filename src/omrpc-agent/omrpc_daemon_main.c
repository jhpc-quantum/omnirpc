static char rcsid[] = "$Id: omrpc_daemon_main.c,v 1.1 2006-01-25 15:59:11 ynaka Exp $";
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

#include <fcntl.h>
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_agent.h"
#include "omrpc_stub.h"

#include "omrpc_daemon_defs.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

short omrpc_stub_version_major,omrpc_stub_version_minor;
short omrpc_stub_init = 0;
char *omrpc_module_name = "omrpc-daemon";
short omrpc_n_entry = 0;
NINF_STUB_INFO *omrpc_stub_info_table[1];

extern int omrpc_mxio_flag;
extern char *omrpc_sched_type;

unsigned short daemon_port = 0;
static char *omrpc_req_name(int req);
void omrpc_stub_INIT_Daemon(int argc, char * argv[]);
void omrpc_stub_EXIT_Daemon();
void do_ninf_protocol(int fd);



void sig_child(int signo)
{
  pid_t pid;
  int stat;

  while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
    omrpc_prf("child %d terminated\n", pid);
  }
  return ;
}

int main(int argc, char *argv[])
{

    /* specialized variables for daemon */
    int sock_optval = 1;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    int listen_fd, connfd;
    pid_t child_pid;

    /* use same sequence */
    omrpc_stub_INIT_Daemon(argc,argv);

    /* default RR scheduler */
    if(omrpc_daemon_job_type == JOB_DAEMON_RR){
      omrpc_daemon_sched_init();
    }


    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0){
      perror("socket");
      exit(1);
    }

    if( setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   &sock_optval, sizeof(sock_optval)) == -1){
      perror("setsockopt");
      exit(1);
    }
    if( setsockopt(listen_fd, IPPROTO_TCP, TCP_NODELAY,
                   &sock_optval, sizeof(sock_optval)) == -1){
      perror("setsockopt");
      exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(daemon_port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind (listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
      perror("bind");
      exit(1);
    }

    if( listen(listen_fd, 4) == -1){
      perror("listen");
      exit(1);
    }

    signal(SIGCHLD, sig_child);

    for ( ; ; ){
      clilen = sizeof(cliaddr);
      omrpc_prf("wait to connect...\n");
      if((connfd = accept(listen_fd, (struct sockaddr *) &cliaddr, &clilen)) < 0){
        if(errno == EINTR){
          continue;
        } else {
          perror("accept error");
        }
      }

      if(( child_pid = fork()) == 0){
        char  buff[INET_ADDRSTRLEN];
        close(listen_fd);
        omrpc_client_hostname = (char*)inet_ntop(AF_INET,
                                                 (void *)&cliaddr.sin_addr,
                                                 buff, sizeof(buff));
        omrpc_prf("connection from %s, port %d\n", 
                  omrpc_client_hostname,
                  ntohs(cliaddr.sin_port));

        do_ninf_protocol(connfd);
      } else {
        close(connfd);
      }
    }

    return 0;
}


void do_ninf_protocol(int fd)
{
  char request;
  char *path;
  char buf[256];
  char *registry_path;

  int status;

  pid_t pid;

  omrpc_io_handle_t *stub_hp;

  stub_hp = omrpc_io_handle_fd(fd, omrpc_mxio_flag);


 next:
  if(omrpc_debug_flag)
    omrpc_prf("rcv_cmd ...\n");


  request = omrpc_recv_cmd(stub_hp);
  if(request == (char)EOF)
    goto exit;

  if(omrpc_debug_flag)
    omrpc_prf("req=%d(%s)\n",request,omrpc_req_name(request));

  switch(request){
  case OMRPC_AGENT_READ_REG:
    omrpc_recv_strdup(stub_hp,&path);
    omrpc_recv_done(stub_hp);
    registry_path = buf;
    if(path == NULL) {
      omrpc_get_default_path(buf,REGISTRY_FILE);
    } else {
      strcpy(buf,path);
      strcat(buf,"/");
      strcat(buf,REGISTRY_FILE);
    }
    
    if(omrpc_debug_flag){
      omrpc_prf("OMRPC_AGENT_READ_REG: file='%s'\n",registry_path);
    }
    fd = open(registry_path,O_RDONLY);
    if(fd < 0){
      omrpc_send_cmd(stub_hp,OMRPC_ACK_NG);
    } else {
      omrpc_send_cmd(stub_hp,OMRPC_ACK_OK);
      omrpc_send_file(stub_hp,fd);
    }
    omrpc_send_done(stub_hp);
    close(fd);
    if(path != NULL) omrpc_free(path);
    break;
    
  case OMRPC_AGENT_EXEC:
    {
      omrpc_rex_proc_t *rp;
      short port_num;
      
      omrpc_recv_strdup(stub_hp,&path);
      omrpc_recv_short(stub_hp,&port_num,1);
      omrpc_recv_done(stub_hp);
      
      if(omrpc_debug_flag){
        omrpc_prf("OMRPC_AGENT_EXEC: file='%s' port=%d\n",path,port_num);
      }
      
      /* default action: fork */
      rp = omrpc_daemon_submit(path,omrpc_client_hostname,port_num);

      if(rp == NULL) omrpc_send_cmd(stub_hp,OMRPC_ACK_NG);
      else omrpc_send_cmd(stub_hp,OMRPC_ACK_OK);
      omrpc_send_done(stub_hp);
      
      omrpc_free(path);
    }
    break;
    
  case OMRPC_AGENT_READ_DONE:
    omrpc_recv_done(stub_hp);
    omrpc_fatal("AGENT_READ_DONE, not mxio mode");
    /* not reply */
    break;
    
  case OMRPC_MRG_KILL:        /* kill omrpc-daemon */
    omrpc_recv_done(stub_hp);
    omrpc_io_handle_close(stub_hp);
    goto exit;
    
  case OMRPC_MRG_INIT:
  default:
    omrpc_fatal("daemon, unknown command=%d\n",request);
  }
  
  /* clean up pid */
  while((pid = wait3(&status,WNOHANG,NULL)) != 0){
    if(pid == -1){
      if (errno == ECHILD) {
        break;
      } else if (errno == EINTR) {
        continue;
      }
      perror("wait3");
      omrpc_fatal("wait3 is failed");
    } else
      omrpc_daemon_proc_killed(pid);
  }

  goto next;

exit:
    if(omrpc_debug_flag) omrpc_prf("omrpc-daemon is terminated\n");
    exit(0);
}

static char *omrpc_req_name(int req)
{
    switch(req){
    case OMRPC_MRG_INIT: return "INIT";
    case OMRPC_AGENT_READ_REG: return "READ_REG";
    case OMRPC_AGENT_EXEC: return "EXEC";
    case OMRPC_MRG_KILL: return "KILL";
    case OMRPC_AGENT_READ_DONE: return "READ_DONE";
    default: return "???";
    }
}


void
omrpc_stub_INIT_Daemon(int argc, char * argv[])
{

    char *hostname;
    int ac;
    char *arg;
    extern char *omrpc_my_prog_name;
    extern char *omrpc_agent_registry_path;

    omrpc_my_prog_name = strdup(argv[0]);
    hostname = NULL;

#ifdef not
    omrpc_debug_flag = TRUE;
#endif

    for(ac = 1; ac < argc; ac++){
        if(omrpc_debug_flag)
            omrpc_prf("arg[%d]=%s\n",ac,argv[ac]);
        arg = argv[ac];
        if(strcmp(arg,"-p") == 0 || strcmp(arg,"-port") == 0){
            if(++ac >= argc) goto arg_err;
            daemon_port = atoi(argv[ac]);
        } else if(strcmp(arg,"-debug") == 0 || strcmp(arg,"-d") == 0){
            omrpc_debug_flag = TRUE;
            omrpc_prf("stub debug ON ...\n");
        } else if(strcmp(arg,"-reg") == 0){ /* registry path */
            if(++ac >= argc) goto arg_err;
            omrpc_agent_registry_path = strdup(argv[ac]);
        } else if(strcmp(arg,"-workpath") == 0){
            if(++ac >= argc) goto arg_err;
            omrpc_working_path = argv[ac];
        } else {
            omrpc_prf("ac=%d", ac);
            omrpc_fatal("bad arg=%s",arg);
            goto arg_err;
        }
    }

    if(daemon_port == 0)
      omrpc_prf("port is not specified, default port no 10000 is used.\n");

    if(omrpc_debug_flag)
        omrpc_prf("stub arg port=%d, host=%s\n",
                  daemon_port, hostname == NULL ? "*none*": hostname);

    omrpc_io_init();

    if(omrpc_debug_flag) omrpc_prf("stub init End ...\n");
    return;

arg_err:
    omrpc_fatal("bad args, exit...");
    exit(1);
}

