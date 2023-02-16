/*
 * $Id: tlog-omrpc.h,v 1.1.1.1 2004-11-03 21:01:18 yoshihiro Exp $
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

#ifndef _TLOG_OMRPC_H
#define _TLOG_OMRPC_H

#define MAX_THREADS 256

typedef enum tlog_type {
    TLOG_UNDEF = 0,    /* undefined */
    TLOG_END = 1,      /* END*/
    TLOG_START = 2,
    TLOG_RAW = 3,      /* RAW information */
    TLOG_EVENT = 4,
    TLOG_EVENT_IN = 5,
    TLOG_EVENT_OUT = 6,
    TLOG_FUNC_IN = 7,
    TLOG_FUNC_OUT = 8,

    TLOG_RPC_IN = 9,
    TLOG_RPC_OUT = 10,

    TLOG_INVOKE_IN = 11,
    TLOG_INVOKE_OUT = 12,

    TLOG_CALL_IN = 13,
    TLOG_CALL_OUT = 14,

    TLOG_CALL_INIT_IN = 15,
    TLOG_CALL_INIT_OUT = 16,

    TLOG_STUB_INFO_IN = 17,
    TLOG_STUB_INFO_OUT = 18,

    TLOG_SEND_ARG_IN = 19,
    TLOG_SEND_ARG_OUT = 20,

    TLOG_RECV_ARG_IN = 21,
    TLOG_RECV_ARG_OUT = 22,

    TLOG_INIT_AGENT_IN = 23,
    TLOG_INIT_AGENT_OUT = 24,

    TLOG_CALL_IN_EVENT = 25,
    TLOG_CALL_OUT_EVENT = 26,
    TLOG_WAIT_IN_EVENT = 27,
    TLOG_WAIT_OUT_EVENT = 28,

    TLOG_INIT_MODULE_EVENT = 29,
    TLOG_CALL_ASYNC_EVENT = 30,

    TLOG_END_END
} TLOG_TYPE;


void tlog_INIT_AGENT_IN(void);
void tlog_INIT_AGENT_OUT(void);

void tlog_RPC_IN(int id);
void tlog_RPC_OUT(int id);

void tlog_INVOKE_IN(int id);
void tlog_INVOKE_OUT(int id);

void tlog_CALL_IN(int id);
void tlog_CALL_OUT(int id);

void tlog_CALL_INIT_IN(int id);
void tlog_CALL_INIT_OUT(int id);

void tlog_STUB_INFO_IN(int id);
void tlog_STUB_INFO_OUT(int id);

void tlog_SEND_ARG_IN(int id);
void tlog_SEND_ARG_OUT(int id);

void tlog_RECV_ARG_IN(int id);
void tlog_RECV_ARG_OUT(int id);

/* event */
void tlog_CALL_IN_EVENT(void);
void tlog_CALL_OUT_EVENT(void);
void tlog_CALL_ASYNC_EVENT(void);
void tlog_INIT_MODULE_EVENT(void);

void tlog_WAIT_IN_EVENT(void);
void tlog_WAIT_OUT_EVENT(void);

void tlog_init(char *);
void tlog_finalize(void);

#endif /* _TLOG_OMRPC_H */

