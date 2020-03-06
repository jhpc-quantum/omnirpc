// $Id: tlogInit.java,v 1.1.1.1 2004-11-03 21:01:38 yoshihiro Exp $
// $Release: omnirpc-2.0.1 $
// $Copyright:
//  OmniRPC Version 1.0
//  Copyright (C) 2002-2004 HPCS Laboratory, University of Tsukuba.
//  
//  This software is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License version
//  2.1 published by the Free Software Foundation.
//  
//  Please check the Copyright and License information in the files named
//  COPYRIGHT and LICENSE under the top  directory of the OmniRPC Grid PRC 
//  System release kit.
//  
//  
//  $

import java.awt.*;

public class tlogInit {
  
  final static byte RPC_IN = 9;
  final static byte RPC_OUT = 10;
  final static byte INVOKE_IN = 11;
  final static byte INVOKE_OUT = 12;
  final static byte CALL_IN = 13;
  final static byte CALL_OUT = 14;
  final static byte CALL_INIT_IN = 15;
  final static byte CALL_INIT_OUT = 16;
  final static byte STUB_INFO_IN = 17;
  final static byte STUB_INFO_OUT = 18;
  final static byte SEND_ARG_IN = 19;
  final static byte SEND_ARG_OUT = 20;
  final static byte RECV_ARG_IN = 21;
  final static byte RECV_ARG_OUT = 22;
  final static byte INIT_AGENT_IN = 23;
  final static byte INIT_AGENT_OUT = 24;
  final static byte CALL_IN_EVENT = 25;
  final static byte CALL_OUT_EVENT = 26;
  final static byte WAIT_IN_EVENT = 27;
  final static byte WAIT_OUT_EVENT = 28;
  final static byte INIT_MODULE_EVENT = 29;
  final static byte CALL_ASYNC_EVENT = 30;

  public static void init() {
    tlogData.setMaxType(31);
    tlogData.defineEvent2Type(RPC_IN,RPC_OUT,"rex active",Color.white);
    tlogData.defineEvent2Type(INVOKE_IN,INVOKE_OUT,"rex invoke",Color.yellow);
    tlogData.defineEvent2Type(CALL_IN,CALL_OUT,"rex call",Color.red);
    tlogData.defineEvent2Type(CALL_INIT_IN,CALL_INIT_OUT,
			      "rex init",Color.pink);
    tlogData.defineEvent2Type(STUB_INFO_IN,STUB_INFO_OUT,
			    "stub info",Color.green);
    tlogData.defineEvent2Type(SEND_ARG_IN,SEND_ARG_OUT,
			    "send args",Color.cyan);
    tlogData.defineEvent2Type(RECV_ARG_IN,RECV_ARG_OUT,
			    "recv args",Color.magenta);

    tlogData.defineEvent2Type(INIT_AGENT_IN,INIT_AGENT_OUT,
			    "init agent",Color.yellow);

    tlogData.defineEventType(CALL_IN_EVENT,"call start",Color.magenta);
    tlogData.defineEventType(CALL_OUT_EVENT,"call done",Color.blue);
    tlogData.defineEventType(WAIT_IN_EVENT,"wait start",Color.pink);
    tlogData.defineEventType(WAIT_OUT_EVENT,"wait done",Color.green);

    tlogData.defineEventType(INIT_MODULE_EVENT,"wait start",Color.cyan);
    tlogData.defineEventType(CALL_ASYNC_EVENT,"call async",Color.red);
  }
}


