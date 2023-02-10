static char rcsid[] = "$Id: omrpc_host.c,v 1.2 2006-01-25 16:06:17 ynaka Exp $";
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
#include "omrpc_defs.h"
#include "omrpc_host.h"
#include "omrpc_rpc.h"
#include "omrpc_agent.h"
#include "omrpc_mon.h"
#include "xml_node.h"

/*
 * host data management:
 */
#define OMRPC_STUB_HASH_SIZE 0x100
#define MAXPATHLEN 256
#define MAX_LINE_LEN 4096

#define STR_EQ(s1,s2) (strcmp(s1,s2) == 0)

int omrpc_tlog_flag = FALSE;

int omrpc_n_hosts = 0;
omrpc_host_t *omrpc_hosts[OMRPC_MAX_HOSTS];

int omrpc_n_modules = 0;
omrpc_module_t *omrpc_modules[OMRPC_MAX_MODULES];

#define OMRPC_STUB_HASH_MASK (OMRPC_STUB_HASH_SIZE-1)
static omrpc_stub_entry_t *omrpc_stub_hash_table[OMRPC_STUB_HASH_SIZE];

/* local prototype */
static void omrpc_setup_agent(int host_id);
static void omrpc_put_stub_entry(omrpc_host_t *host,char *entry_name,
                                 char *module_name,char *path);
static omrpc_module_t *omrpc_get_module(char *module_name);

static void omrpc_xml_alloc_host(xml_node x);

int omrpc_get_host_id(omrpc_host_t *host);

omrpc_host_t *omrpc_alloc_host()
{
    return (omrpc_host_t *)omrpc_malloc(sizeof(omrpc_host_t));
}

#if 1

void omrpc_read_hostfile(char *host_file)
{
    FILE *fp;
    char fName[MAXPATHLEN];
    xml_node xp, x = NULL;
    xml_node_list lp;
    char *attr_v;

    if(host_file == NULL){
        host_file = fName;
        strcpy(host_file,HOST_FILE);        /* check current directory */
        fp = fopen(host_file,"r");
        if(fp == NULL){
            omrpc_get_default_path(host_file,HOST_FILE);
            fp = fopen(host_file,"r");
        }
    } else {
      fp = fopen(host_file,"r");
    }

    if(fp == NULL){
      omrpc_prf("cannot open hostfile '%s',"
                "use localhost as a single rmote host\n", host_file);
      omrpc_xml_alloc_host(NULL);
      return;
    }

    if(omrpc_debug_flag) omrpc_prf("hosts file= '%s'\n",host_file);

    xp = xml_read_document(fp);
    fclose(fp);

    if(omrpc_debug_flag) xml_node_print(xp,stdout);

    for(lp = xp->child; lp != NULL; lp = lp->next){
        x = lp->item;
        if(IS_XML_ELEMENT(x,"OmniRpcConfig")) break;
    }
    if(lp == NULL){
        omrpc_error("no 'OmniRpcConfig' found in hostfile\n");
    }
    for(lp = x->child; lp != NULL; lp = lp->next){
        if(IS_XML_ELEMENT(lp->item,"Host"))
            omrpc_xml_alloc_host(lp->item);
        else if(IS_XML_ELEMENT(lp->item,"Debug"))
            omrpc_debug_flag = TRUE;
        else if(IS_XML_ELEMENT(lp->item,"Log")){
            omrpc_tlog_flag = TRUE;
            if((attr_v = xml_attr_value(lp->item,"file")) == NULL)
                attr_v = OMRPC_TLOG_FNAME;
            tlog_init(attr_v);
        } else if (IS_XML_ELEMENT(lp->item, "Monitor")){
            omrpc_monitor_flag = TRUE;
            if((attr_v = xml_attr_value(x,"host")) != NULL){
                omrpc_monitor_target_host = attr_v;
            }
            if((attr_v = xml_attr_value(x,"port")) != NULL){
                omrpc_monitor_port = atoi(attr_v);
            }
        } else if (IS_XML_ELEMENT(lp->item, "TimeOut")){
            if((attr_v = xml_attr_value(x,"second")) != NULL){
                omrpc_timeout_sec = atoi(attr_v);
            }
        } else if(lp->item->type != XML_TEXT){
            omrpc_error("bad xml element in hostfile");
        }
    }
    if (omrpc_n_hosts <= 0) {
        omrpc_error("No valid host in hostfile '%s'. exit.\n",
                    host_file);
    }
}

static void omrpc_xml_alloc_host(xml_node x)
{
    omrpc_host_t *hostp;
    xml_node_list lp;
    char *attr_v;

    hostp = omrpc_alloc_host();

    /* set default */
    hostp->max_jobs = 1;
    hostp->n_jobs = 0;
    hostp->port_num = 0;
    hostp->registry_path = NULL;
    hostp->agent_path = OMRPC_DIR;
    hostp->fork_type = FORK_RSH;
    hostp->mxio_flag = FALSE;
    hostp->job_sched_type = JOB_SCHED_FORK;
    hostp->user_name = NULL;
    hostp->working_path = WORKING_PATH_DEFAULT;

    if(x == NULL){
        hostp->name = "localhost";
        goto ret;
    }

    if((attr_v = xml_attr_value(x,"name")) == NULL){
        omrpc_error("name is not specified in 'Host'\n");
    }
    hostp->name = attr_v;

    if((attr_v = xml_attr_value(x,"user")) != NULL){
      hostp->user_name = attr_v;
    }

    lp = x->child;
    for( ; lp != NULL; lp = lp->next)
        if(lp->item->type != XML_TEXT) break; /* skip */
    if(lp != NULL && IS_XML_ELEMENT(lp->item,"Agent")){
        x = lp->item;
        if((attr_v = xml_attr_value(x,"invoker")) == NULL)
            omrpc_error("Agent invoker is not specified");
        if(STR_EQ(attr_v,"rsh"))
            hostp->fork_type = FORK_RSH;
        else if(STR_EQ(attr_v,"ssh"))
            hostp->fork_type = FORK_SSH;
        else if(STR_EQ(attr_v,"globus") || STR_EQ(attr_v,"gram")){
            hostp->fork_type = FORK_GRAM;
            omrpc_use_globus = TRUE;
        } else if(STR_EQ(attr_v, "server") || STR_EQ(attr_v, "daemon")){
          /* this is for specialized version of OmniRPC for Windows */
            hostp->fork_type = FORK_DAEMON;
        } else if(STR_EQ(attr_v, "mpi")){
	  // Count the number of nodes in "registry_path/nodes" which must be equal to "hostfile" of MPI
            char fname[MAXPATHLEN];
            char linebuf[MAX_LINE_LEN + 1];
            FILE *fp;
            hostp->fork_type = FORK_MPI;
            if(hostp->registry_path != NULL){
              strcpy(fname,hostp->registry_path);
              strcat(fname,"/nodes");
            }else{
              omrpc_get_default_path(fname,"nodes");
	    }
            fp = fopen(fname,"r");
            if(fp == NULL){
              if(omrpc_debug_flag) omrpc_prf("nodes file '%s' not found\n",fname);
              return;
            }
            hostp->mpi_nodes_total=0;
            while(fgets(linebuf,MAX_LINE_LEN,fp) != NULL){
              if(strlen(linebuf)<=0) continue;
              hostp->mpi_nodes_total++;
	    }
            hostp->mpi_nodes_used  = 1; // Should it be '2' ??
            if(omrpc_debug_flag) omrpc_prf("%d nodes are found and set as MPI_UNIVERSE_SIZE\n",hostp->mpi_nodes_total);
        } else omrpc_error("Agent invoker '%s' is unknown!",attr_v);

        if((attr_v = xml_attr_value(x,"mxio")) != NULL){
            if(STR_EQ(attr_v,"on") || STR_EQ(attr_v,"yes"))
                hostp->mxio_flag = TRUE;
            else if(STR_EQ(attr_v,"off") || STR_EQ(attr_v,"no"))
                hostp->mxio_flag = FALSE;
            else omrpc_error("Agent mxio must be on/off");
        }

        if((attr_v = xml_attr_value(x,"path")) != NULL)
            hostp->agent_path = attr_v;

        if((attr_v = xml_attr_value(x, "port")) != NULL){
            hostp->port_num = atoi(attr_v);
        }

        for(lp = lp->next ; lp != NULL; lp = lp->next)
            if(lp->item->type != XML_TEXT) break; /* skip */
	
    }

    if(lp != NULL && IS_XML_ELEMENT(lp->item,"JobScheduler")){
        x = lp->item;
        if((attr_v = xml_attr_value(x,"type")) != NULL){
            if(STR_EQ(attr_v,"fork"))
                hostp->job_sched_type = JOB_SCHED_FORK;
            else if(STR_EQ(attr_v,"rr") || STR_EQ(attr_v,"round-robin"))
                hostp->job_sched_type = JOB_SCHED_RR;
            else if(STR_EQ(attr_v,"pbs"))
                hostp->job_sched_type = JOB_SCHED_PBS;
            else if(STR_EQ(attr_v,"sge"))
                hostp->job_sched_type = JOB_SCHED_SGE;
            else omrpc_error("JobScheduler type '%s' is unknown",attr_v);
        }

        if((attr_v = xml_attr_value(x,"maxjob")) != NULL)
            hostp->max_jobs = atoi(attr_v);

        for(lp = lp->next ; lp != NULL; lp = lp->next)
            if(lp->item->type != XML_TEXT) break; /* skip */
    }

    if(lp != NULL && IS_XML_ELEMENT(lp->item,"Registry")){
        x = lp->item;
        if((attr_v = xml_attr_value(x,"path")) == NULL)
            omrpc_error("Registry path is not specifed");
        hostp->registry_path = attr_v;

        for(lp = lp->next ; lp != NULL; lp = lp->next)
            if(lp->item->type != XML_TEXT) break; /* skip */
    }

    if(lp != NULL && IS_XML_ELEMENT(lp->item,"Description")){
        for(lp = lp->next ; lp != NULL; lp = lp->next)
            if(lp->item->type != XML_TEXT) break; /* skip */
    }

    if(lp != NULL && IS_XML_ELEMENT(lp->item,"WorkingPath")){
        x = lp->item;
        if((attr_v = xml_attr_value(x, "path")) == NULL){
            omrpc_error("Working path is not specified");
        }
        hostp->working_path = attr_v;
        for(lp = lp->next ; lp != NULL; lp = lp->next)
            if(lp->item->type != XML_TEXT) break; /* skip */
    }
    if(lp != NULL) omrpc_error("unknown element in Host");

ret:
    if (omrpc_debug_flag) {
        omrpc_prf("host='%s' jobs=%d reg=%s\n",
                  hostp->name, hostp->max_jobs,
                  (hostp->registry_path == NULL) ?
                  "<null>": hostp->registry_path);
        omrpc_prf("host='%s' forktype='%d', port_num='%d'\n",
                  hostp->name, hostp->fork_type, hostp->port_num);
    }
    omrpc_hosts[omrpc_n_hosts++] = hostp;
}

#else /* old hostfile */

void omrpc_read_hostfile(char *host_file)
{
    FILE *fp;
    char linebuf[MAX_LINE_LEN + 1];
    char fName[MAXPATHLEN];
    char *cp;
    char host_name[256];
    char dir[256];
    char max_jobs_buf[10],io_type_buf[10];
    int i,max_jobs;
    omrpc_host_t *hostp;

    if(host_file == NULL){
        host_file = fName;
        omrpc_get_default_path(host_file,HOST_FILE);
    }

    if(omrpc_debug_flag) omrpc_prf("hosts file= '%s'\n",host_file);

    fp = fopen(host_file,"r");
    if(fp == NULL){
        omrpc_error("cannot open hosts file '%s'\n",host_file);
    }

    omrpc_n_hosts = 0;
    while(fgets(linebuf,MAX_LINE_LEN,fp) != NULL){
        cp = linebuf;
        while(*cp != '\0' && isspace((int)(*cp))) cp++;
        if(*cp == '#' || *cp == '\0' ||
           *cp == '\r' || *cp == '\n')
            continue;
        i = sscanf(cp,"%s %s %s %s",host_name,max_jobs_buf,dir,io_type_buf);
        if(i <= 0) {
            omrpc_prf("bad hosts file '%s'\n",host_file);
            exit(1);
        }
        hostp = omrpc_alloc_host();
        if(i >= 1){
            hostp->name = strdup(host_name);
            hostp->max_jobs = 1;	/* default */
            hostp->n_jobs = 0;
            hostp->registry_path = NULL;
        }
        if(i >= 2 && strcmp(max_jobs_buf,"*")!= 0){
            max_jobs = atoi(max_jobs_buf);
            hostp->max_jobs = max_jobs;
        }
        if(i >= 3 && strcmp(dir,"*") != 0)
            hostp->registry_path = strdup(dir);
        if(i >= 4 && strcmp(io_type_buf,"*") != 0){
            /* rsh, rsh-mxio, ssh, ssh-mxio, globus, globus-mxio */
            if(strcmp(io_type_buf,"rsh") == 0){
                hostp->fork_type = FORK_RSH;
                hostp->mxio_flag = FALSE;
            } else if(strcmp(io_type_buf,"rsh-mxio") == 0){
                hostp->fork_type = FORK_RSH;
                hostp->mxio_flag = TRUE;
            } else if(strcmp(io_type_buf,"rsh-rr") == 0){
                hostp->fork_type = FORK_RSH;
                hostp->job_sched_type = JOB_SCHED_RR;
            } else if(strcmp(io_type_buf,"ssh") == 0){
                hostp->fork_type = FORK_SSH;
                hostp->mxio_flag = FALSE;
            } else if(strcmp(io_type_buf,"ssh-mxio") == 0){
                hostp->fork_type = FORK_SSH;
                hostp->mxio_flag = TRUE;
            } else if(strcmp(io_type_buf,"ssh-rr") == 0){
                hostp->fork_type = FORK_SSH;
                hostp->job_sched_type = JOB_SCHED_RR;
            } else if(strcmp(io_type_buf,"globus") == 0){
                hostp->fork_type = FORK_GRAM;
                hostp->mxio_flag = FALSE;
                omrpc_use_globus = TRUE;
            } else if(strcmp(io_type_buf,"globus-mxio") == 0){
                hostp->fork_type = FORK_GRAM;
                hostp->mxio_flag = TRUE;
                omrpc_use_globus = TRUE;
            } else if(strcmp(io_type_buf,"globus-rr") == 0){
                hostp->fork_type = FORK_GRAM;
                hostp->job_sched_type = JOB_SCHED_RR;
                omrpc_use_globus = TRUE;
            } else
                omrpc_fatal("bad io_type '%s'\n",io_type_buf);
        }

        if (omrpc_debug_flag) {
            omrpc_prf("host='%s' jobs=%d reg=%s\n",
                      hostp->name, hostp->max_jobs,
                      (hostp->registry_path == NULL) ?
                      "<null>": hostp->registry_path);
        }
        omrpc_hosts[omrpc_n_hosts++] = hostp;
    }
    fclose(fp);

    if (omrpc_n_hosts <= 0) {
        omrpc_error("No valid host in machine file '%s'. exit.\n",
                    host_file);
    }
}
#endif


void omrpc_init_hosts()
{
    int i;
    for(i = 0; i < omrpc_n_hosts; i++) omrpc_setup_agent(i);
}

/*
 * for omrpc_agent
 */
static void omrpc_setup_agent(int host_id)
{
    omrpc_host_t *hostp;
    omrpc_rpc_t *rp;
    char *cp;
    int r;
    char path[512];

    hostp = omrpc_hosts[host_id];
    snprintf(path,sizeof(path),"%s/bin/%s",hostp->agent_path,OMRPC_AGENT_NAME);
    if(omrpc_tlog_flag) tlog_INIT_AGENT_IN();
    if(hostp->fork_type == FORK_DAEMON){
      // connect to the daemon
      rp = omrpc_exec_on_host_daemon(hostp,path);
    } else {
      rp = omrpc_exec_on_host(hostp,path);
    }
    if(omrpc_tlog_flag) tlog_INIT_AGENT_OUT();

    if(rp == NULL){
        omrpc_fatal("cannot exec omrpc-agent on '%s'\n",hostp->name);
    }
    hostp->agent_rp = rp;

    /* read registery */
    r = omrpc_agent_read_registry(rp->hp,hostp->registry_path,&cp);
    if(r != OMRPC_ACK_OK){
        omrpc_prf("warning: cannot read registry on '%s'\n",
                omrpc_hosts[host_id]->name);
        return;
    }

    omrpc_parse_registry(host_id,cp);
    omrpc_free(cp);
}

void omrpc_parse_registry(int host_id, char *cp)
{
    char linebuf[MAX_LINE_LEN];
    char path[256];
    char module_name[50],entry_name[50];
    char *cq;
    int n,i;

    if(omrpc_debug_flag)
        omrpc_prf("registry on host_id %d:\n %s\n",host_id,cp);

    n = 0;
    while(1){
        cq = linebuf;
        while(*cp != '\n' && *cp != '\r'){
            if(*cp == 0) goto ret;
            *cq++= *cp++;
        }
        cp++;
        *cq = '\0';

        cq = linebuf;
        while(isspace(*cq)) cq++;	/* skip space */
        if(*cq == '#' || *cq == '\0') continue;

        i = sscanf(cq,"%s %s %s",module_name,entry_name, path);
        if(i != 3) omrpc_fatal("bad registry on host '%s'",
                               omrpc_hosts[host_id]->name);
        omrpc_put_stub_entry(omrpc_hosts[host_id],entry_name,module_name,path);
        n++;
    }
ret:
    if(n == 0){
        omrpc_prf("warning: no stub is registered on '%s'\n",
                omrpc_hosts[host_id]->name);
    }
}

omrpc_module_t *omrpc_find_module(char *module_name)
{
    int i;
    omrpc_module_t *mp = NULL;

    for(i = 0; i < omrpc_n_modules; i++){
        mp = omrpc_modules[i];
        if(strcmp(mp->name,module_name) == 0) break;
    }

    if(i == omrpc_n_modules){
      omrpc_fatal("cannot find module %s\n",module_name);
      return NULL;
    }
    return mp;
}

static omrpc_module_t *omrpc_get_module(char *module_name)
{
    int i;
    omrpc_module_t *mp = NULL;

    for(i = 0; i < omrpc_n_modules; i++){
        mp = omrpc_modules[i];
        if(strcmp(mp->name,module_name) == 0) break;
    }
    if(i == omrpc_n_modules){
        if(i >= OMRPC_MAX_MODULES) omrpc_fatal("module table overflow");
        mp = (omrpc_module_t *) omrpc_malloc(sizeof(omrpc_module_t));
        mp->name = strdup(module_name);
        omrpc_modules[omrpc_n_modules++] = mp;
    }
    return mp;
}

static void omrpc_put_stub_entry(omrpc_host_t *host,char *entry_name,
                                 char *module_name,char *path)
{
    int hcode;
    omrpc_stub_entry_t *sp;
    omrpc_module_t *mp;
    omrpc_rex_prog_t *rexp;
    char *cp;

    hcode = 0;
    for(cp = entry_name; *cp != 0; cp++)
        hcode = (hcode << 1) + *cp;
    hcode &= OMRPC_STUB_HASH_MASK;

    mp = omrpc_get_module(module_name);

    for(sp = omrpc_stub_hash_table[hcode]; sp != NULL; sp = sp->next){
        if(strcmp(sp->entry_name,entry_name) == 0 && sp->module == mp)
            break;
    }

    if(sp == NULL){ /* not found, make entry */
        sp = (omrpc_stub_entry_t *)omrpc_malloc(sizeof(omrpc_stub_entry_t));
        sp->entry_name = strdup(entry_name);
        sp->module = mp;
        sp->next = omrpc_stub_hash_table[hcode];
        omrpc_stub_hash_table[hcode] = sp;
    }

    /* put rex prog */
    for(rexp = mp->rex_list; rexp != NULL; rexp = rexp->next){
        if(rexp->host == host){
            if(strcmp(rexp->path, path) != 0){
                omrpc_error("host '%s' has two executables with the same module name '%s:%s'\n",host->name,module_name,entry_name);
            }
            break;
        }
    }

    if(rexp == NULL){
        rexp = (omrpc_rex_prog_t *)omrpc_malloc(sizeof(omrpc_rex_prog_t));
        rexp->host = host;
        rexp->path = strdup(path);
        rexp->next = mp->rex_list;
        mp->rex_list = rexp;
    }

    if(omrpc_debug_flag)
        omrpc_prf("stub_entry(host='%s'): %s.%s %s\n",
                  host->name, sp->module->name,sp->entry_name,path);
}

omrpc_stub_entry_t *omrpc_find_stub_entry(char *entry_name, 
                                          char *module_name)
{
    int hcode;
    char *cp;
    omrpc_stub_entry_t *sp;
    omrpc_module_t *module = NULL;

    if(module_name != NULL){
        module = omrpc_find_module(module_name);
        if(module == NULL) return NULL;      /* module not found */
    }

    hcode = 0;
    for( cp = entry_name ; *cp != 0 ; cp++ )
        hcode = (hcode << 1) + *cp;
    hcode &= OMRPC_STUB_HASH_MASK;

    for(sp = omrpc_stub_hash_table[hcode]; sp != NULL; sp = sp->next){
        if(strcmp(sp->entry_name,entry_name) == 0 &&
           (module == NULL || sp->module == module))
            return sp;
    }
    return NULL;
}

int omrpc_find_host_id(char *host_name)
{
    int i;
    for(i = 0; i < omrpc_n_hosts; i++)
        if(strcmp(omrpc_hosts[i]->name,host_name) == 0)
            return i;
    return -1;
}

omrpc_host_t *omrpc_find_host(char *host_name)
{
    int i;
    for(i = 0; i < omrpc_n_hosts; i++)
        if(strcmp(omrpc_hosts[i]->name,host_name) == 0)
            return omrpc_hosts[i];
    return NULL;
}

int omrpc_host_has_module(int host_id, omrpc_module_t *module)
{
    omrpc_rex_prog_t *rexp;
    for(rexp = module->rex_list; rexp != NULL; rexp = rexp->next)
        if(rexp->host == omrpc_hosts[host_id]) return TRUE;
    return FALSE;
}

char *omrpc_rex_prog_path(omrpc_host_t *host, omrpc_module_t *module)
{
    omrpc_rex_prog_t *rexp;
    for(rexp = module->rex_list; rexp != NULL; rexp = rexp->next)
        if(rexp->host == host) return rexp->path;
    return NULL;
}

char *omrpc_job_sched_type_name(int job_sched_type)
{
    switch(job_sched_type){
    case JOB_SCHED_FORK: 
        return NULL;
    case JOB_SCHED_RR:
        return "rr";
    case JOB_SCHED_PBS:
        return "pbs";
    case JOB_SCHED_SGE:
        return "sge";
    default:
        omrpc_fatal("job_sched_type=%d is not supported",job_sched_type);
        return NULL;
    }
}

int omrpc_get_host_id(omrpc_host_t *host)
{
    int i;
    for(i = 0; i < omrpc_n_hosts; i++){
        if( host == omrpc_hosts[i]){
            return i;
        }
    }
    omrpc_fatal("no hosts exits host=%d, n_hosts=%d", host,omrpc_n_hosts);
    return (-1);
}
