/* 
 * $Id: ninf_stub_info.h,v 1.1.1.1 2004-11-03 21:01:36 yoshihiro Exp $
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
/*********************** ninf_stub_info.h *********************/

#ifndef  _NINF_STUB_INFO_H_
#define  _NINF_STUB_INFO_H_
#include <inttypes.h>

#define MAJOR_VERSION 3
#define MINOR_VERSION 1

/* basic data type */
typedef enum data_type
{
    DT_UNDEF = 0,	/* undefined */
    DT_VOID,
    DT_CHAR,
    DT_SHORT,
    DT_INT,
    DT_LONG,
    DT_LONGLONG,
    DT_UNSIGNED_CHAR,
    DT_UNSIGNED_SHORT,
    DT_UNSIGNED,
    DT_UNSIGNED_LONG,
    DT_UNSIGNED_LONGLONG,
    DT_FLOAT,
    DT_DOUBLE,
    DT_LONG_DOUBLE,
    DT_SCOMPLEX,
    DT_DCOMPLEX,
    DT_STRING_TYPE,
    DT_FILENAME,
    DT_FILEPOINTER,
    DT_FUNC_TYPE,
    BASIC_TYPE_END
} DATA_TYPE;

#define MAX_DATA_TYPE	((int32_t)(BASIC_TYPE_END))

#define DATA_TYPE_NAMES \
{ \
    "DT_UNDEF",	\
    "DT_VOID",\
    "DT_CHAR",\
    "DT_SHORT",\
    "DT_INT",\
    "DT_LONG",\
    "DT_LONGLONG",\
    "DT_UNSIGNED_CHAR",\
    "DT_UNSIGNED_SHORT",\
    "DT_UNSIGNED",\
    "DT_UNSIGNED_LONG",\
    "DT_UNSIGNED_LONGLONG",\
    "DT_FLOAT",\
    "DT_DOUBLE",\
    "DT_LONG_DOUBLE",\
    "DT_SCOMPLEX",\
    "DT_DCOMPLEX",\
    "DT_STRING_TYPE",\
    "DT_FILENAME",\
    "DT_FILEPOINTER",\
    "DT_FUNC_TYPE"\
} 


/* IN/OUT mode specifier */
typedef enum mode_spec
{
    MODE_NONE = 0,
    MODE_IN = 1,	/* default */
    MODE_OUT = 2,
    MODE_INOUT = 3,
    MODE_WORK = 4,      /* mode for work space */
    MODE_CALL_BACK_FUNC = 5  /* call back function */
} MODE_SPEC;

#define MODE_SPEC_NAMES \
{\
    "MODE_NONE",\
    "MODE_IN",\
    "MODE_OUT",\
    "MODE_INOUT",\
    "MODE_WORK",\
    "MODE_CALL_BACK_FUNC" \
}

#define IS_IN_MODE(x)	((int)(x)&1)
#define IS_OUT_MODE(x)	((int)(x)&2)
#define IS_WORK_MODE(x) ((int)(x)&4)

/* maxmum string length */
#define MAX_STRING_LEN	1000

#define MAX_NAME_LEN	100
#define MAX_DIM		10
#define MAX_PARAMS	30
#define NINF_EXPRESSION_LENGTH 20

typedef enum value_type
{
    VALUE_ERROR = -1,
    VALUE_NONE = 0,	/* default */
    VALUE_CONST = 1,	/* default, give by constant */
    VALUE_IN_ARG = 2,	/* specified by IN scalar paramter */
    VALUE_BY_EXPR = 3, 	/* computed by interpreter */
    VALUE_OP = 4,       /* operation code */
    VALUE_END_OF_OP = 5, /* end of expression */
    VALUE_IN_HEADER = 6 /* for interface to NetSolve */
} VALUE_TYPE;

#define VALUE_TYPE_NAMES \
{\
    "VALUE_NONE",\
    "VALUE_CONST",\
    "VALUE_IN_ARG",\
    "VALUE_BY_EXPR",\
    "VALUE_OP",\
    "VALUE_END_OF_OP",\
    "VALUE_IN_HEADER"\
}

/********          supported operators          ********/
#define OP_VALUE_PLUS  1
#define OP_VALUE_MINUS 2
#define OP_VALUE_MUL   3
#define OP_VALUE_DIV   4
#define OP_VALUE_MOD   5
#define OP_VALUE_UN_MINUS   6
#define OP_VALUE_BEKI   7
#define OP_VALUE_EQ     8   /*  ==  */
#define OP_VALUE_NEQ    9   /*  !=  */
#define OP_VALUE_GT    10   /*  >  */
#define OP_VALUE_LT    11   /*  <  */
#define OP_VALUE_GE    12   /*  >=  */
#define OP_VALUE_LE    13   /*  <=  */
#define OP_VALUE_TRY   14   /*  ? : */

typedef struct ninf_expression 
{
    VALUE_TYPE type[NINF_EXPRESSION_LENGTH];
    int32_t val[NINF_EXPRESSION_LENGTH];
} NINF_EXPRESSION;

struct ninf_param_desc {
    enum data_type param_type;	/* argument type */
    enum mode_spec param_inout;	/* IN/OUT */
    int32_t ndim;			/* number of dimension */
    struct {
	VALUE_TYPE size_type;
	int32_t size;
	NINF_EXPRESSION size_exp;
	VALUE_TYPE start_type;
	int32_t start;
	NINF_EXPRESSION start_exp;
	VALUE_TYPE end_type;
	int32_t end;
	NINF_EXPRESSION end_exp;
	VALUE_TYPE step_type;
	int32_t step;
	NINF_EXPRESSION step_exp;
    } dim[MAX_DIM];
};

#define MAT_STYLE_C       0
#define MAT_STYLE_FORTRAN 1

#define BACKEND_NORMAL   0
#define BACKEND_MPI      1

typedef struct ninf_stub_information
{
    int32_t version_major,version_minor;	/* protocol version */
    int32_t info_type;		/* information type: fortran style of C */  
    char module_name[MAX_NAME_LEN];	/* module name */
    char entry_name[MAX_NAME_LEN];	/* entry name */
    int32_t nparam;
    struct ninf_param_desc * params;
    int32_t order_type;
    NINF_EXPRESSION order;
    char * description;
    char shrink;		/* boolean: specify server side shrink */
    char backend;		/* MPI or not */
    short index;
} NINF_STUB_INFO;

/* prototype */
NINF_STUB_INFO *ninf_new_stub_info(void);
struct ninf_param_desc *ninf_new_param_desc(int32_t num);

#endif /* _NINF_STUB_INFO_H_ */
