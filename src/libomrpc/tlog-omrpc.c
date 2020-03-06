static char rcsid[] = "$Id: tlog-omrpc.c,v 1.1.1.1 2004-11-03 21:01:18 yoshihiro Exp $";
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
#include "tlog.h"

void tlog_INIT_AGENT_IN() { tlog_log(0,TLOG_INIT_AGENT_IN); }
void tlog_INIT_AGENT_OUT() { tlog_log(0,TLOG_INIT_AGENT_OUT); }

void tlog_RPC_IN(int id) { tlog_log(id,TLOG_RPC_IN); }
void tlog_RPC_OUT(int id) { tlog_log(id,TLOG_RPC_OUT); }

void tlog_INVOKE_IN(int id) { tlog_log(id,TLOG_INVOKE_IN); }
void tlog_INVOKE_OUT(int id) { tlog_log(id,TLOG_INVOKE_OUT); }

void tlog_CALL_IN(int id) { tlog_log(id,TLOG_CALL_IN); }
void tlog_CALL_OUT(int id) { tlog_log(id,TLOG_CALL_OUT); }

void tlog_CALL_INIT_IN(int id) { tlog_log(id,TLOG_CALL_INIT_IN); }
void tlog_CALL_INIT_OUT(int id) { tlog_log(id,TLOG_CALL_INIT_OUT); }

void tlog_STUB_INFO_IN(int id) { tlog_log(id,TLOG_STUB_INFO_IN); }
void tlog_STUB_INFO_OUT(int id) { tlog_log(id,TLOG_STUB_INFO_OUT); }

void tlog_SEND_ARG_IN(int id) { tlog_log(id,TLOG_SEND_ARG_IN); }
void tlog_SEND_ARG_OUT(int id) { tlog_log(id,TLOG_SEND_ARG_OUT); }

void tlog_RECV_ARG_IN(int id) { tlog_log(id,TLOG_RECV_ARG_IN); }
void tlog_RECV_ARG_OUT(int id) { tlog_log(id,TLOG_RECV_ARG_OUT); }

/* event */
void tlog_CALL_IN_EVENT() { tlog_log(0,TLOG_CALL_IN_EVENT); }
void tlog_CALL_OUT_EVENT() { tlog_log(0,TLOG_CALL_OUT_EVENT); }
void tlog_CALL_ASYNC_EVENT() { tlog_log(0,TLOG_CALL_ASYNC_EVENT); }
void tlog_INIT_MODULE_EVENT() { tlog_log(0,TLOG_INIT_MODULE_EVENT); }

void tlog_WAIT_IN_EVENT() { tlog_log(0,TLOG_WAIT_IN_EVENT); }
void tlog_WAIT_OUT_EVENT() { tlog_log(0,TLOG_WAIT_OUT_EVENT); }

