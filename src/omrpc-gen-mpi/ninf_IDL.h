/* 
 * $Id: ninf_IDL.h,v 1.1.1.1 2004-11-03 21:01:20 yoshihiro Exp $
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
/* 
 * internal expression data structure 
 */
#define MAX_CODE 128

enum expr_code {
	ERROR_NODE = 0,
	LIST = 1,
	IDENT = 2,
	STRING_CONSTANT = 3,
	INT_CONSTANT = 4,
	LONG_CONSTANT = 5,
	FLOAT_CONSTANT = 6,
	BASIC_TYPE_NODE = 7,
	MODE_SPEC_NODE = 8,

	PLUS_EXPR = 35,
	MINUS_EXPR = 37,
	UNARY_MINUS_EXPR = 39,
	MUL_EXPR = 40,
	BEKI_EXPR = 41,
	DIV_EXPR = 42,
	MOD_EXPR = 44,

	EQ_EXPR = 50,
	NEQ_EXPR = 51,
	GT_EXPR = 52,
	LT_EXPR = 53,
	GE_EXPR = 54,
	LE_EXPR = 55,
	TRY_EXPR = 56,

	POINTER_REF = 66,
	ARRAY_REF = 67,
	CALLBACK_FUNC = 68
};

enum symbol_type 
{
    S_IDENT=0,	/* default */
    S_TYPE,
    S_CLASS,
    S_KEYWORD
};

/* symbol and symbol table */
typedef struct symbol
{
    struct symbol *s_next;		/* backet chain */
    char *s_name;
    enum symbol_type s_type/*:8*/;	/* symbol type, KEYWORD, NAME, etc ... */
    short int s_value;
} * SYMBOL;

SYMBOL find_symbol();

#define SYM_NAME(sp)	((sp)->s_name)

/* de-syntax program is represented by this data structure. */
typedef struct expression_node
{
    enum expr_code e_code/*:8*/;
    short int e_lineno;		/* line number this node created */
    union 
      {
	  struct list_node *e_lp;
	  SYMBOL e_sym;		/* e_code == NAME, TYPE, ...  */
	  char *e_str;		/* e_code == STRING */
	  long int e_ival;		/* e_code == INT_CONSTANT */
	  double *e_fval;	/* e_code == FLOAT_CONSTANT */
      } v;
} * expr;

#define EXPR_CODE(x)	((x)->e_code)	
#define EXPR_SYM(x)	((x)->v.e_sym)
#define EXPR_STR(x)	((x)->v.e_str)
#define EXPR_FLOAT(x)	(*((x)->v.e_fval))
#define EXPR_LIST(x)	((x)->v.e_lp)
#define EXPR_INT(x)	((x)->v.e_ival)
#define EXPR_TYPE(x)	((DATA_TYPE)((x)->v.e_ival))
#define EXPR_MODE_SPEC(x)	((MODE_SPEC)((x)->v.e_ival))
#define EXPR_LINENO(x)	((x)->e_lineno)

/* list data structure, which is ended with NULL */
typedef struct list_node
{
    struct list_node *l_next;
    expr l_item;
} *list;

#define LIST_NEXT(lp)	((lp)->l_next)
#define LIST_ITEM(lp)	((lp)->l_item)
#define FOR_ITEMS_IN_LIST(lp,x) \
  if(x != NULL) for(lp = EXPR_LIST(x); lp != NULL ; lp = LIST_NEXT(lp))

#define EXPR_ARG1(x)	LIST_ITEM(EXPR_LIST(x))
#define EXPR_ARG2(x)	LIST_ITEM(LIST_NEXT(EXPR_LIST(x)))
#define EXPR_ARG3(x)	LIST_ITEM(LIST_NEXT(LIST_NEXT(EXPR_LIST(x))))
#define EXPR_ARG4(x) LIST_ITEM(LIST_NEXT(LIST_NEXT(LIST_NEXT(EXPR_LIST(x)))))

/* constructor */
expr list0(), list1(), list2(), list3(), list4();
expr list_put_last();

expr make_enode();

char *save_str( char *);
double * save_float(float);

extern struct expr_code_info
{
    char *code_name;
    char *operator_name;
} expr_code_info[];

#define EXPR_CODE_NAME(code)	expr_code_info[code].code_name
#define EXPR_CODE_SYMBOL(code)   expr_code_info[code].operator_name


expr compile_required(expr x);
expr compile_backend(expr x);
expr compile_shrink(expr x);
expr compile_language(expr x);
expr compile_calcorder(expr x);
