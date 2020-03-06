/* 
 * $Id: omrpc_gen.h,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $
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
#include <stdarg.h>
#include <limits.h>
#include "ninf_stub_info.h"
#include "ninf_IDL.h"

#define DEFAULT_REX_NAME "rex.c"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define THIS_INFO_TYPE	0

#define MAX_STUBS 	200
#define MAX_GLOBALS 	100

struct stub_gen_entry 
{
    SYMBOL ent_id;	/* entry name */
    int nparam;
    struct param_gen_desc{
      enum data_type param_type;	/* argument type */
      enum mode_spec param_inout;	/* IN/OUT */
      SYMBOL param_id;		/* parameter name */
      int ndim;			/* number of dimension */
      struct dim_gen_desc{
	expr size_expr;		/* size */
	expr range_exprs;	/* range */
	VALUE_TYPE size_type;
	int size;		
	NINF_EXPRESSION size_exp;
	VALUE_TYPE start_type;
	int start;
	NINF_EXPRESSION start_exp;
	VALUE_TYPE end_type;
	int end;

	NINF_EXPRESSION end_exp;
	VALUE_TYPE step_type;
	int step;
	NINF_EXPRESSION step_exp;
      } dim[MAX_DIM];
    } params[MAX_PARAMS];
    char *body;
    expr body_expr;
    char *description;
    char *required;
    int order_type;
    NINF_EXPRESSION order;
    char *language;
    int backend;
    int shrink;
};

extern struct stub_gen_entry stubs[MAX_STUBS];
extern int n_stubs;

extern char *globals[MAX_GLOBALS];
extern int n_globals;

extern char *current_module;

#define STR_EQ(s1,s2)	(strcmp(s1,s2) == 0)

extern char *program;	/* this program */
 
extern int lineno;
extern char *source_file_name,*output_file_name;
extern FILE *source_file,*output_file;

extern int debug_flag;
extern FILE *debug_fp;

extern expr	read_rest_of_body(int flag);

extern void	compile_MODULE(expr x);
extern void	compile_OPTIONS(expr x);
extern void	compile_GLOBALS(expr x);
extern void	compile_LIBRARY(expr x);
extern void	compile_COMPILE_OPTIONS(expr x);
extern void	compile_FORTRANFORMAT(expr x);
extern void	compile_interface(expr def, expr desc, expr option_list, expr body);

extern void	generate_rpc_executable(FILE *fp);
extern void	dump_stubs(void);
extern void	init_expr_code_info(void);
extern void	initialize_lex(void);

extern void	expr_print(expr x, FILE *fp);
extern void	error(char *fmt, ...);
extern void	error_at_node(expr x, char *fmt, ...);
extern void	fatal(char *fmt, ...);
extern void     warning(char *fmt, ...);

extern char *	basic_type_name(DATA_TYPE type);
extern char *	mode_spec_name(MODE_SPEC mode);
