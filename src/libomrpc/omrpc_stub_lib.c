static char rcsid[] = "$Id: omrpc_stub_lib.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $";
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
#include "omrpc_stub.h"

#include "omrpc_mon.h"
#ifdef USE_MPI
#include "omrpc_mpi.h"
#include "omrpc_mpi_io.h"
#endif /* USE_MPI */

#include "myx_master_wrapper.h"
#ifdef USE_GLOBUS
#include "omrpc_globus.h"
#endif

/* stub information of this stub */
short omrpc_stub_magic = OMRPC_STUB_MAGIC;
extern short omrpc_stub_version_major,omrpc_stub_version_minor;
extern short omrpc_stub_init;
extern char *omrpc_module_name;
extern short omrpc_n_entry;
extern NINF_STUB_INFO *omrpc_stub_info_table[];

omrpc_io_handle_t  *omrpc_stub_hp;
#ifdef USE_MPI
omrpc_mpi_handle_t *omrpc_stub_mp;
#endif /* USE_MPI */
char *omrpc_client_hostname;
int omrpc_mxio_flag;
char *omrpc_sched_type = NULL;
char *omrpc_agent_registry_path = NULL;
pthread_t omrpc_monitor_tid;
pthread_attr_t omrpc_monitor_t_attr;

static any_t stub_args[MAX_PARAMS];
static NINF_STUB_INFO *stub_info;

static void omrpc_show_stub_info();

#ifdef USE_FT
// heartbeat trace with pthread 
static void *omrpcm_fault_tolerance(void *arg);
pthread_t       omrpc_ft_tid;
pthread_attr_t  omrpc_ft_t_attr;
pthread_mutex_t omrpc_ft_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  omrpc_ft_cond =PTHREAD_COND_INITIALIZER;
struct omrpc_ft_info{
  unsigned short port;
  char           *client_name;
};
struct omrpc_ft_info    *finfo;
struct sched_param      fparam;
extern char fjmpi_is_master;

#endif

/* stub program libaray:
 *
 *  omrpc_stub_INIT(argc,argv)
 *	Initalize all variable, make connection etc...
 *
 *  omrpc_stub_REQ()
 *	take request packet return 1 if NINF_REQ_CALL,
 *	return 0 if NINF_REQ_KILL, otherwise loop.
 *
 *  omrpc_stub_SET_ARG(cp,arg_i)
 *	Set values to arg.
 *
 *  omrpc_stub_END()
 *      Output results of OUT parameters, and clean up.
 *
 *  omrpc_stub_EXIT()
 *	end the stub program.
 */

void
omrpc_stub_INIT(int argc, char * argv[])
{
    unsigned short port = 0;
    char *hostname;
    int   ac,fd,myrank;
    omrpc_io_handle_t *hp = NULL;
    char *arg;
    extern char *omrpc_my_prog_name;
    int port_forwarding_flag = FALSE;
    extern char *omrpc_agent_registry_path;
#ifdef USE_FT
    static int omrpc_init_ft=FALSE;
#endif

    omrpc_my_prog_name = strdup(argv[0]);
    hostname = NULL;
#ifdef USE_MPI
    omrpc_stub_mp = NULL;
#endif /* USE_MPI */
    omrpc_stub_hp = NULL;

#if 0
    omrpc_debug_flag = TRUE;
#endif

    if(argc == 1){ /* no argument */
        omrpc_show_stub_info();
        exit(0);
    }

    for(ac = 1; ac < argc; ac++){
        if(omrpc_debug_flag)
            omrpc_prf("arg[%d]=%s\n",ac,argv[ac]);
        arg = argv[ac];
        if(strcmp(arg,"-show") == 0){ /* show interface info */
            omrpc_show_stub_info();
            exit(0);
        } else if(strcmp(arg,"-p") == 0 || strcmp(arg,"-port") == 0){
            if(++ac >= argc) goto arg_err;
            port = atoi(argv[ac]);
        } else if(strcmp(arg,"-h") == 0 || strcmp(arg,"-host") == 0){
            if(++ac >= argc) goto arg_err;
            hostname = argv[ac];
        } else if(strcmp(arg,"-globus") == 0){ /* globus mode */
#ifdef USE_GLOBUS
            omrpc_use_globus = TRUE;
#else
            omrpc_fatal("globus mode is not available");
#endif
        } else if(strcmp(arg,"-debug") == 0 || strcmp(arg,"-d") == 0){
            omrpc_debug_flag = TRUE;
            omrpc_prf("stub debug ON ...\n");
        } else if(strcmp(arg,"-mxio") == 0){
            omrpc_mxio_flag = TRUE;
        } else if(strcmp(arg,"-sched") == 0){	/* scheduler for agent */
            if(++ac >= argc) goto arg_err;
            omrpc_sched_type = argv[ac];
            if(omrpc_debug_flag) omrpc_prf("%s\n",omrpc_sched_type);
        } else if(strcmp(arg,"-reg") == 0){ /* registry path */
            if(++ac >= argc) goto arg_err;
            omrpc_agent_registry_path = strdup(argv[ac]);
        } else if(strcmp(arg,"-port-forwarding") == 0){
            port_forwarding_flag = TRUE;
        } else if(strcmp(arg,"-mon") == 0){
#ifdef USE_MONITOR
            omrpc_monitor_flag = TRUE;
            if(++ac >= argc) goto arg_err;
            omrpc_monitor_target_host = argv[ac];
#else
            omrpc_prf("monitor function is not available\n");
#endif
        } else if(strcmp(arg,"-mon-port") == 0){
#ifdef USE_MONITOR
            if(!omrpc_monitor_flag)
                omrpc_monitor_target_host = hostname;
            if(++ac >= argc) goto arg_err;
            omrpc_monitor_port = atoi(argv[ac]);
#else
            omrpc_prf("monitor function is not available\n");
#endif
        } else if(strcmp(arg,"-workpath") == 0){
            if(++ac >= argc) goto arg_err;
            omrpc_working_path = argv[ac];
        } else {
            omrpc_prf("ac=%d", ac);
            omrpc_fatal("bad arg=%s",arg);
            goto arg_err;
        }
    }

#ifdef USE_MPI
    if(strcmp(omrpc_sched_type,"MPI")==0){
      short short_buf[6];
      MPI_Status status;

      omrpc_io_init();
      omrpc_client_hostname = hostname;
      omrpc_stub_mp = (omrpc_mpi_handle_t *)omrpcm_handle_create(); 
      MPI_Comm_get_parent(&(omrpc_stub_mp->comm));
#ifdef USE_FT
      // recv port name for FJMPI_connect & accpet
      fjmpi_is_master=0;
      MPI_Recv(omrpc_stub_mp->ft_pname, MPI_MAX_PORT_NAME, MPI_CHAR, 0, 0, omrpc_stub_mp->comm, &status);
#endif
      omrpc_stub_mp->tag = 0;
      short_buf[0] = omrpc_stub_magic;
      short_buf[1] = omrpc_stub_version_major;
      short_buf[2] = omrpc_stub_version_minor;
      short_buf[3] = omrpc_stub_init;
      short_buf[4] = omrpc_n_entry;
      short_buf[5] = getpid();
      omrpcm_send_short(omrpc_stub_mp, short_buf, 6, 0, omrpc_stub_mp->tag++);

#ifdef USE_FT
      // Initialize for fault tolerant
      if(!omrpc_init_ft){
        MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
        if(myrank==0){
	  if(port==0) omrpc_fatal("port is not specified");
	  finfo=(struct omrpc_ft_info *)omrpc_malloc(sizeof(struct omrpc_ft_info));
	  finfo->port=port;
	  finfo->client_name=omrpc_client_hostname;
          pthread_attr_init(&omrpc_ft_t_attr);
 	  pthread_create(&omrpc_ft_tid,&omrpc_ft_t_attr,omrpcm_fault_tolerance,finfo);
        }
        omrpc_init_ft          = TRUE;
        omrpc_stub_mp->ft_send = TRUE;
      }
#endif
      if(omrpc_debug_flag) omrpc_prf("stub init End ...\n");
      return;
    } // if(strcmp(omrpc_sched_type,"MPI")==0)
#endif /* USE_MPI */
    
    if(port == 0) omrpc_fatal("port is not specified");

    if(omrpc_debug_flag)
        omrpc_prf("stub arg port=%u,%u host=%s\n",
                   port, port+16384, hostname == NULL ? "*none*": hostname);
    omrpc_io_init();
  
    if(omrpc_use_globus){
#ifdef USE_GLOBUS
        omrpc_init_globus_io();
        hp = omrpc_globus_io_handle(omrpc_mxio_flag);
        omrpc_globus_io_connect(hostname,port,hp->port);
#endif
    } else {
        /* alway create simple io in remote side */
        if(port_forwarding_flag)
            fd = omrpc_io_connect("localhost",port);
        else
            fd = omrpc_io_connect(hostname,port);
        hp = omrpc_io_handle_fd(fd,omrpc_mxio_flag);
    }
    omrpc_io_handle_byte_order(hp,FALSE);

    omrpc_send_short(hp,&omrpc_stub_magic,1);
    omrpc_send_short(hp,&omrpc_stub_version_major,1);
    omrpc_send_short(hp,&omrpc_stub_version_minor,1);
    omrpc_send_short(hp,&omrpc_stub_init,1);
    omrpc_send_short(hp,&omrpc_n_entry,1);
    omrpc_send_done(hp);

    omrpc_stub_hp = hp;
    omrpc_client_hostname = hostname;
#ifdef USE_MONITOR
    if(omrpc_monitor_flag){
	omrpc_monitor_init();
	pthread_attr_init(&omrpc_monitor_t_attr);
	pthread_create(&omrpc_monitor_tid, &omrpc_monitor_t_attr, omrpc_monitor_handler,NULL);
    }
#endif

    if(omrpc_debug_flag) omrpc_prf("stub init End ...\n");
    return;

arg_err:
    omrpc_fatal("bad args, exit...");
    exit(1);
}

static void omrpc_show_stub_info()
{
    int i;

    printf("module '%s' contains %d function%s.\n",
	   omrpc_module_name,omrpc_n_entry,
	   (omrpc_n_entry <= 1) ? "":"s");
    for(i = 0; i < omrpc_n_entry; i++)
	ninf_print_stub_info(stdout,omrpc_stub_info_table[i]);
}

int
omrpc_stub_REQ()
{
    char request;
    int i;
    short index;
    char *p;
    struct ninf_param_desc *dp;
    ninf_array_shape_info array_shape;
    char name[MAX_NAME_LEN];

    if(omrpc_debug_flag) omrpc_prf("stub req start ...\n");
    
 next:
    if(omrpc_debug_flag) omrpc_prf("reading request ...\n");

    /* read request */
#ifdef USE_FT
//    pthread_mutex_lock(&omrpc_ft_mutex);
//    pthread_cond_wait(&omrpc_ft_cond,&omrpc_ft_mutex);
#endif
#ifdef USE_MPI
    if(strcmp(omrpc_sched_type,"MPI")==0){
        omrpcm_recv_char(omrpc_stub_mp, &request, 1, 0, omrpc_stub_mp->tag++);
    }else{
#endif /* USE_MPI */
        request = omrpc_recv_cmd(omrpc_stub_hp);
#ifdef USE_MPI
    }
#endif /* USE_MPI */

#ifdef USE_FT
//    pthread_mutex_unlock(&omrpc_ft_mutex);
#endif
    if(omrpc_debug_flag) omrpc_prf("request = %d\n",request);
    if(request == (char)EOF){
        goto killed;
    }


    switch(request){
    case OMRPC_REQ_KILL:
    killed:
#ifdef USE_MPI
        if(strcmp(omrpc_sched_type,"MPI")==0){
#ifdef USE_FT
            if(omrpc_stub_mp->ft_pname) omrpc_free(omrpc_stub_mp->ft_pname);
#endif
            omrpc_free(omrpc_stub_mp);
            omrpc_stub_mp=NULL;
        }else{
#endif /* USE_MPI */
	    omrpc_io_handle_close(omrpc_stub_hp);
#ifdef USE_MPI
        }
#endif /* USE_MPI */
	return -1;	/* exit loop */

    case OMRPC_REQ_CALL:
#ifdef USE_MPI
        if(strcmp(omrpc_sched_type,"MPI")==0){ // MpiBackend 
	    // get request function index 
            omrpcm_recv_short(omrpc_stub_mp, &index, 1, 0, omrpc_stub_mp->tag++);
	    if(omrpc_debug_flag) omrpc_prf("requested call index=%d\n",index);
	    stub_info = omrpc_stub_info_table[index];
            // receive scalar args
	    ninfm_recv_scalar_args(omrpc_stub_mp,stub_info,stub_args,FALSE);
            // receive vector data 
            for (i = 0; i < stub_info->nparam; i++){
                dp = &stub_info->params[i];
                if(dp->ndim == 0) continue;         // scalar 

                // compute the number of elemnts 
                ninf_set_array_shape(dp,stub_info,stub_args,&array_shape);
                // allocate memory for work array 
                p = (char *)omrpc_malloc(array_shape.total_count*
                                         data_type_size(dp->param_type));
                if(omrpc_debug_flag)
                    omrpc_prf("alloc work size=0x%x addr=%p\n",
                              array_shape.total_count*data_type_size(dp->param_type),p);

                stub_args[i].p = p; // set argument 

                if(!IS_IN_MODE (dp->param_inout)) continue;

                if(omrpc_debug_flag) omrpc_prf("stub: recv_array(%d)\n",i);
                ninfm_recv_array(omrpc_stub_mp,p,&array_shape,0);
            }
            if(omrpc_debug_flag) omrpc_prf("REQ_CALL end\n");
        }else{
#endif /* USE_MPI */
	    // get request function index 
   	    omrpc_recv_short(omrpc_stub_hp,&index,1);
	    if(omrpc_debug_flag) omrpc_prf("requested call index=%d\n",index);
	    stub_info = omrpc_stub_info_table[index];
            // receive scalar args
	    ninf_recv_scalar_args(omrpc_stub_hp,stub_info,stub_args,FALSE);

            // receive vector data 
            for (i = 0; i < stub_info->nparam; i++){
                dp = &stub_info->params[i];
                if(dp->ndim == 0) continue;         // scalar 

                // compute the number of elemnts 
                ninf_set_array_shape(dp,stub_info,stub_args,&array_shape);

                // allocate memory for work array 
                p = (char *)omrpc_malloc(array_shape.total_count*
                                         data_type_size(dp->param_type));
                if(omrpc_debug_flag)
                    omrpc_prf("alloc work size=0x%x addr=%p\n",
                              array_shape.total_count*data_type_size(dp->param_type),p);

                stub_args[i].p = p; // set argument 
    
                if(!IS_IN_MODE (dp->param_inout)) continue;

                if(omrpc_debug_flag) omrpc_prf("stub: recv_array(%d)\n",i);
                ninf_recv_array(omrpc_stub_hp,p,&array_shape);
            }
            if(omrpc_debug_flag) omrpc_prf("REQ_CALL end\n");
            omrpc_recv_done(omrpc_stub_hp);
#ifdef USE_MPI
	}
#endif /* USE_MPI */
	return index;		/* continue */

    case OMRPC_REQ_STUB_INFO:
#ifdef USE_MPI
        if(strcmp(omrpc_sched_type,"MPI")==0){
            omrpcm_recv_str(omrpc_stub_mp, name, 0, omrpc_stub_mp->tag++); 
        }else{
#endif /* USE_MPI */
	    omrpc_recv_string(omrpc_stub_hp,name);
	    omrpc_recv_done(omrpc_stub_hp);
#ifdef USE_MPI
        }
#endif /* USE_MPI */
	if(omrpc_debug_flag){
	    omrpc_prf("requested stub: name = %s\n",name);
	}

	/* search stubs */
	for(i = 0; i < omrpc_n_entry; i++){
	    if(strcmp(name,omrpc_stub_info_table[i]->entry_name) == 0) 
		break;
	}

#ifdef USE_MPI
        if(strcmp(omrpc_sched_type,"MPI")==0){
            if(i == omrpc_n_entry){
                char ack=OMRPC_ACK_NG;
                omrpcm_send_char(omrpc_stub_mp, &ack, 1, 0, omrpc_stub_mp->tag++);
                goto next;
            }
            omrpcm_send_stub_info(omrpc_stub_mp,omrpc_stub_info_table[i]);
        }else{
#endif /* USE_MPI */
	    if(i == omrpc_n_entry){
                omrpc_send_cmd(omrpc_stub_hp,OMRPC_ACK_NG);
                omrpc_send_done(omrpc_stub_hp);
                goto next;
            }
            omrpc_send_stub_info(omrpc_stub_hp,omrpc_stub_info_table[i]);
#ifdef USE_MPI
        }
#endif /* USE_MPI */
	goto next;
	
    case OMRPC_REQ_STUB_INFO_BY_INDEX:
	omrpc_recv_short(omrpc_stub_hp,&index,1);
	omrpc_recv_done(omrpc_stub_hp);
	if(omrpc_debug_flag){
	    omrpc_prf("requested stub: index = %d\n",index);
	}
	if(index >= omrpc_n_entry){
	    omrpc_send_cmd(omrpc_stub_hp,OMRPC_ACK_NG);
	    omrpc_send_done(omrpc_stub_hp);
	    goto next;
	}
	omrpc_send_stub_info(omrpc_stub_hp,omrpc_stub_info_table[index]);
	goto next;

    case OMRPC_REQ_STUB_INFO_LOCAL:           /* for omrpc_local_exec */
	omrpc_recv_string(omrpc_stub_hp,name);
	omrpc_recv_done(omrpc_stub_hp);
	if(omrpc_debug_flag){
	    omrpc_prf("requested stub: name = %s\n",name);
	}

	/* search stubs */
	for(i = 0; i < omrpc_n_entry; i++){
	    if(strcmp(name,omrpc_stub_info_table[i]->entry_name) == 0) 
		break;
	}

	if(i == omrpc_n_entry){
	    omrpc_send_cmd(omrpc_stub_hp,OMRPC_ACK_NG);
	    omrpc_send_done(omrpc_stub_hp);
	    goto next;
	} 

    send_stub_local:
	omrpc_send_cmd(omrpc_stub_hp,OMRPC_ACK_OK);

	/* send binary */
	stub_info = omrpc_stub_info_table[i];
	omrpc_send_byte(omrpc_stub_hp,(char *)stub_info,sizeof(*stub_info));
	for(i = 0; i < stub_info->nparam; i++){
	    omrpc_send_byte(omrpc_stub_hp,(char *)&(stub_info->params[i]),
			    sizeof(struct ninf_param_desc));
	}
	omrpc_send_done(omrpc_stub_hp);
	goto next;
	
    case OMRPC_REQ_STUB_INFO_LOCAL_BY_INDEX:
	omrpc_recv_short(omrpc_stub_hp,&index,1);
	omrpc_recv_done(omrpc_stub_hp);
	if(omrpc_debug_flag){
	    omrpc_prf("requested stub: index = %d\n",index);
	}
	if(index >= omrpc_n_entry){
	    omrpc_send_cmd(omrpc_stub_hp,OMRPC_ACK_NG);
	    omrpc_send_done(omrpc_stub_hp);
	    goto next;
	}
	i = index;
	goto send_stub_local;
#ifdef USE_FT
     case OMRPC_STOP_HB:
        if(omrpc_debug_flag) omrpc_prf("reqested: stop heart beat\n");
        omrpc_stub_mp->ft_send = FALSE;
        goto next;
#endif
    }
    omrpc_fatal("STUB ERROR: unknown request %d\n",request);
    return 0;
}

void
omrpc_stub_SET_ARG(void  *cp,int arg_i)
{
    struct ninf_param_desc *dp;
    any_t *ap;
    
    dp = &(stub_info->params[arg_i]);
    ap = &stub_args[arg_i];

    if(omrpc_debug_flag)
	omrpc_prf("Ninf_stub_SET_ARG(%d)=0x%x, %p\n",
		  arg_i,ap->i,ap->p);

    if (dp->ndim == 0){
	switch(dp->param_type){
	case DT_CHAR:
	    *((char *)cp) = ap->c;
	    break;
	case DT_SHORT:
	    *((short *)cp) = ap->s;
	    break;
	case DT_INT:
	    *((int *)cp) = ap->i;
	    break;
	case DT_LONG:
	    *((long *)cp) = ap->l;
	    break;
	case DT_UNSIGNED_CHAR:
	    *((unsigned char *)cp) = ap->uc;
	    break;
	case DT_UNSIGNED_SHORT:
	    *((unsigned short *)cp) = ap->s;
	    break;
	case DT_UNSIGNED:
	    *((unsigned int *)cp) = ap->ui;
	    break;
	case DT_UNSIGNED_LONG:
	    *((unsigned long *)cp) = ap->l;
	    break;
	case DT_FLOAT:
	    *((float *)cp) = ap->f;
	    break;
	case DT_DOUBLE:
	    *((double *)cp) = ap->d;
	    break;
	case DT_LONG_DOUBLE:
	    *((long double *)cp) = ap->ld;
	    break;
	case DT_UNSIGNED_LONGLONG:
	    *((unsigned long long *)cp) = ap->ull;
	    break;
	case DT_LONGLONG:
	    *((long long *)cp) = ap->ll;
	    break;
	case DT_SCOMPLEX:
	    *((float _Complex *)cp) = ap->fc;
	    break;
	case DT_DCOMPLEX:
	    *((double _Complex *)cp) = ap->dc;
	    break;
	case DT_STRING_TYPE:
	    *((char **)cp) = ap->p;
	    break;
        case DT_FILENAME:
            *((char **)cp) = ap->p;
            break;
        case DT_FILEPOINTER:
            *((FILE **)cp) = ap->p;
            break;
	default:
	    omrpc_fatal("unknown data type");
	    break;
	}
    } else *((char **)cp) = ap->p;	/* pointer */
}

void omrpc_stub_BEGIN()
{
    if(omrpc_debug_flag) omrpc_prf("Ninf_stub_BEGIN\n");
}

/* 
 * flush output result 
 */
void
omrpc_stub_END()
{
    struct ninf_param_desc *dp;
    int nparam,i;
    ninf_array_shape_info array_shape;

    nparam = stub_info->nparam;

    if(omrpc_debug_flag) omrpc_prf("Ninf_stub_END begin\n");

    if(omrpc_stub_hp){
        omrpc_send_cmd(omrpc_stub_hp,OMRPC_ACK_OK);

        /* ninf_send_scalar_args(omrpc_stub_hp,stub_info,stub_args,FALSE); */

        if(omrpc_debug_flag) omrpc_prf("Ninf_stub_END send out args\n");

        /* send vector data */
        for (i = 0; i < stub_info->nparam; i++){
	    dp = &stub_info->params[i];
	    if(dp->ndim == 0)                  continue;  // scalar
	    if(!IS_OUT_MODE (dp->param_inout)) continue;
            // compute the number of elemnts 
	    ninf_set_array_shape(dp,stub_info,stub_args,&array_shape);
	    if(omrpc_debug_flag) omrpc_prf("Ninf_stub_END send array(%d)\n",i);
	    ninf_send_array(omrpc_stub_hp,stub_args[i].p,&array_shape);
        }
    
        omrpc_send_done(omrpc_stub_hp);
#ifdef USE_MPI
    }else{ // MPI 
        char ack=OMRPC_ACK_OK;
        omrpcm_send_char(omrpc_stub_mp,&ack,1,0,OMRPC_END_STUB_TAG);
        if(omrpc_debug_flag) omrpc_prf("Ninf_stub_END send out args\n");
        /* send vector data */
        for (i = 0; i < stub_info->nparam; i++){
            dp = &stub_info->params[i];
            if(dp->ndim == 0)                  continue;  // scalar
            if(!IS_OUT_MODE (dp->param_inout)) continue;
            // compute the number of elemnts 
            ninf_set_array_shape(dp,stub_info,stub_args,&array_shape);
            if(omrpc_debug_flag) omrpc_prf("Ninf_stub_END send array(%d)\n",i);
            ninfm_send_array(omrpc_stub_mp,stub_args[i].p,&array_shape,0);
        }
#endif /* USE_MPI */
    }
    

    /* in case of Initialize, skip bellow memory cleann up*/
    /* clean up memory (need ?) */
    for(i = 0; i < nparam; i++){
        dp = &stub_info->params[i];
        if(dp->ndim == 0) continue; 	/* scalar */
        if(omrpc_debug_flag)
            omrpc_prf("freeing %d th param, address %p\n", i,
                      stub_args[i].p);
        omrpc_free(stub_args[i].p);		/* free area */
    }

    if(omrpc_debug_flag) omrpc_prf("Ninf_stub_END: end\n");
}

void
omrpc_stub_EXIT()
{
    /* flush and exit */
    if(omrpc_debug_flag) omrpc_prf("Ninf_stub_EXIT\n");
#ifdef USE_MONITOR
    if(omrpc_monitor_flag){
      pthread_cancel(omrpc_monitor_tid);
    }
#endif
    omrpc_io_finalize();
#ifdef USE_GLOBUS
    if(omrpc_use_globus) omrpc_finalize_globus_io();
#endif

    /* delete files for fileio */
    omrpc_io_fileio_unlink_files();

    exit(0);
}

void
omrpc_stub_EXIT_MPI()
{
    int myrank;
    /* flush and exit */
    if(omrpc_debug_flag) omrpc_prf("Ninf_stub_EXIT\n");
#ifdef USE_MONITOR
    if(omrpc_monitor_flag){
      pthread_cancel(omrpc_monitor_tid);
    }
#endif
    omrpc_io_finalize(); // do nothing .. 
#ifdef USE_GLOBUS
    if(omrpc_use_globus) omrpc_finalize_globus_io();
#endif

#ifdef USE_FT
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    if(myrank==0){
      pthread_cancel(omrpc_ft_tid);
    }
    free(finfo);
#endif 

    /* delete files for fileio */
    omrpc_io_fileio_unlink_files();
}


/* port specific operation */
void omrpc_recv_done_port(omrpc_io_port_t *port)
{
    /* nothing to do on stub/mgr side */
}

/* dummy */
void omrpc_io_lock() {  }
void omrpc_io_unlock() {  }


#ifdef USE_FT
/* 
  Simple Fault Tolerance HeartBeat using socket 
  2013-02-11 / 2013-04-08
  M.TSUJI@PRiSM-UVSQ
 */
static void *omrpcm_fault_tolerance(void *arg)
{
  int    fd;
  char   ack=OMRPC_MPI_ALIVE;
  double et0,et1,et2;
  unsigned short port;
  useconds_t     sleep_sec;
  struct omrpc_ft_info *finfo=arg;
  omrpc_io_handle_t    *hp   =NULL;

  sleep_sec=1000000*OMRPC_MPI_HBINTVL;
  et0=MPI_Wtime();
  if(omrpc_debug_flag) omrpc_prf("pthread for fault tolerance is forked\n");
 
  // Establish connection and send first ack to client
  pthread_mutex_lock(&omrpc_ft_mutex);
  fd=omrpc_io_connect(finfo->client_name,finfo->port);
  hp=omrpc_io_handle_fd(fd,omrpc_mxio_flag);

  omrpc_io_handle_byte_order(hp,FALSE);
  omrpc_send_char(hp,&ack,1);
  omrpc_send_done(hp);

  if(omrpc_debug_flag) omrpc_prf("connection for fault tolerance. port=%d, client=%s etime=%e\n",finfo->port,finfo->client_name,MPI_Wtime()-et0);

  pthread_cond_signal(&omrpc_ft_cond);
  pthread_mutex_unlock(&omrpc_ft_mutex);

  sched_yield();
  et1=MPI_Wtime();
  while(1){
    sched_yield(); 
    et2=MPI_Wtime();
    if(et2-et1>OMRPC_MPI_HBINTVL && omrpc_stub_mp->ft_send){
      pthread_mutex_lock(&omrpc_ft_mutex);
      //if(omrpc_debug_flag) omrpc_prf("send hb %e %e (sec) \n",et2-et1,et2-et0);
      omrpc_send_char(hp,&ack,1);
      omrpc_send_done(hp);
      //if(omrpc_debug_flag) omrpc_prf("send hb ... %e done\n",MPI_Wtime()-et0);
      pthread_cond_signal(&omrpc_ft_cond);
      pthread_mutex_unlock(&omrpc_ft_mutex);
      et1=et2;
    }
    else if(et2-et1>OMRPC_MPI_HBINTVL){
      if(omrpc_debug_flag) omrpc_prf("do NOT send hb %e %e (sec) \n",et2-et1,et2-et0); 
      et1=et2;
    }
    else{
      //usleep(sleep_sec);
    }
    sched_yield(); 
  }
} /* omrpcm_fault_tolerance */
#endif

#ifdef USE_FT
// An API to ask master whether rex[id] is alive or not.
int OmniRpcMpiAskHandleAlive(short id)
{
  int  myrank;
  char status;
  MPI_Status stat;
  MPI_Comm   comm;

  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  if(myrank!=0){ // rank!=0はハンドラを持っていないのでgetする
    MPI_Comm_get_parent(&comm);
  }else{
    comm = omrpc_stub_mp->comm;
  }
  //omrpcm_send_short(omrpc_stub_mp,&id,1,0,OMRPC_ASKALIVE_TAG);
  //omrpcm_recv_char(omrpc_stub_mp,&status,1,0,OMRPC_ASKALIVE_TAG);
  // Checkなしでsend/recvする
  MPI_Send(&id, 1, MPI_SHORT, 0, OMRPC_ASKALIVE_TAG, comm);
  MPI_Recv(&status, 1, MPI_CHAR, 0, OMRPC_ASKALIVE_TAG, comm, &stat);

  if(status==OMRPC_MPI_ALIVE) return 1;
  else                        return 0;
} /*  OmniRpcMpiAskHandleAlive */
#endif




