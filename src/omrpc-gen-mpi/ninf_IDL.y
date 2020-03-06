/* 
 * $Id: ninf_IDL.y,v 1.1.1.1 2004-11-03 21:01:20 yoshihiro Exp $
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
 * Ninf IDL grammer
 */

%token SERROR 		/* syntax error mark */
%token CONSTANT 	/* contant: integer, ... */
%token IDENTIFIER 	/* identifier */
%token RELOP            /* relative operator */
%token STRING		/* string */
%token TYPENAME

/* reserved words, etc */
/* type and mode keywords */
%token TYPE MODE 

/* keywords */
%token MODULE COMPILE_OPTIONS GLOBALS DEFINE CALLS REQUIRED BACKEND SHRINK CALCORDER LIBRARY FORTRANFORMAT LANGUAGE

/* precedence rules */
%left '?' ':'
%left '|'		/* bitwise operation */
%left '^'		/* bitwise operation */
%left '&'		/* bitwise operation */
%left RELOP
%left LSHIFT RSHIFT	/* shift operation */
%left '+' '-'		/* add and minus */ 
%left '*' '/' '%'	/* mul and div , mod */
%right '!' '~'		/* unary operation */
%right UNARY
%left UNARY_HIGH_PRIORITY
%left '[' '(' 

%{
#include <ctype.h>
#include <stdlib.h>
#include "omrpc_gen.h"

extern void     yyerror(char *s);
extern int      yylex(void);

typedef union 
{
    expr val;
    enum expr_code code;
} yystype;
#define YYSTYPE yystype
%}

/* define types */
%type <val> interface_definition parameter_list parameter id_list
%type <val> decl_specifier MODE TYPE type_specifier declarator
%type <val> interface_body globals_body opt_string required backend shrink calcorder language option_list decl_option
%type <val> expr expr_or_null unary_expr 
%type <val> primary_expr IDENTIFIER CONSTANT STRING TYPENAME RELOP 
%type <val> range_spec
%type <val> REQUIRED BACKEND SHRINK CALCORDER LANGUAGE

%start program

%%
/* program toplevel */
program:/* empty */
  	| declaration_list
	;

declaration_list: 
	  declaration
	| declaration_list declaration
	;

declaration:
          MODULE IDENTIFIER ';'
	{ compile_MODULE($2); }
        | COMPILE_OPTIONS STRING ';'
	{ compile_COMPILE_OPTIONS($2); }
        | GLOBALS globals_body
	{ compile_GLOBALS($2); }
        | LIBRARY STRING ';'
	{ compile_LIBRARY($2); }
        | FORTRANFORMAT STRING ';'
        { compile_FORTRANFORMAT($2);}
	| DEFINE interface_definition opt_string option_list interface_body
	{ compile_interface($2,$3,$4,$5); }
	| error
	;

option_list:
	/* empty */ {$$ = NULL;}
        | decl_option
          { $$ = list1(LIST, $1);} 
        | option_list decl_option 
	  { $$ = list_put_last($1, $2);}     

decl_option:
        required
          { $$ = $1;}
        | backend
          { $$ = $1;}
        | shrink
          { $$ = $1;}
        | calcorder
          { $$ = $1;}
        | language
          { $$ = $1;}
	;

interface_definition: 
  	  IDENTIFIER '(' parameter_list ')'
	{ $$ = list2(LIST,$1,$3); }
	;

parameter_list: 
	parameter
	{ $$ = list1(LIST,$1); }
	| parameter_list ',' parameter
	{ $$ = list_put_last($1,$3); }
	;

parameter: decl_specifier declarator
	{ $$ = list2(LIST,$1,$2); }
	;

decl_specifier: 
	type_specifier
	{ $$ = list2(LIST,NULL,$1); }
	| MODE
	{ $$ = list2(LIST,$1,NULL); }
	| MODE type_specifier
	{ $$ = list2(LIST,$1,$2); }
	| type_specifier MODE
	{ $$ = list2(LIST,$2,$1); }
	| type_specifier MODE type_specifier
	{ $$ = list2(LIST,$2,list2(LIST,$1,$3)); } 
	;

type_specifier:
	TYPE
	| TYPE TYPE
	{ $$ = list2(LIST,$1,$2); }
	| TYPE TYPE TYPE	/* ex. unsigned long int */
	{ $$ = list2(LIST,$1,list2(LIST,$2,$3)); }
  	| TYPENAME
	;

declarator:
	  IDENTIFIER
	| '(' declarator ')'
	{ $$ = $2; }
	| declarator '['expr_or_null ']'
	{ $$ = list3(ARRAY_REF,$1,$3,NULL); }
	| declarator '['expr_or_null ':' range_spec ']'
	{ $$ = list3(ARRAY_REF,$1,$3,$5); }
	| '*' declarator
	{ $$ = list1(POINTER_REF,$2); }
	;

range_spec:
  	  expr
	{ $$ = list3(LIST,NULL,$1,NULL); }
	| expr ',' expr
	{ $$ = list3(LIST,$1,$3,NULL); }
	| expr ',' expr ',' expr
	{ $$ = list3(LIST,$1,$3,$5); }
	;

opt_string: 
	/* empty */ { $$ = NULL; }
	| STRING 
	;

required:
	REQUIRED STRING
	{ $$ = compile_required($2); }
	;

backend:
	BACKEND STRING
        { $$ = compile_backend($2);}
	;

shrink:
	SHRINK STRING
        { $$ = compile_shrink($2);}
	;

language:
	LANGUAGE STRING
	{ $$ = compile_language($2); }
	;

calcorder:
        CALCORDER expr
        { $$ = compile_calcorder($2);}
        ;

interface_body:
	/* body in C declaration */
	'{' { $$ = read_rest_of_body(TRUE); } 
	| CALLS opt_string IDENTIFIER '(' id_list ')' ';'
	  { $$ = list3(LIST,$2,$3,$5); }
	;

globals_body:
	'{' { $$ = read_rest_of_body(FALSE); } 
	;

id_list: IDENTIFIER
	  { $$ = list1(LIST,$1); }
	| id_list ',' IDENTIFIER
	  { $$ = list_put_last($1,$3); }
	;


/* index descrition */
expr_or_null: 
	expr 
	| { $$ = NULL; } /* null */
	;

expr:		 
	 unary_expr
	| expr '/' expr
	{ $$ = list2(DIV_EXPR,$1,$3); }
	| expr '%' expr
	{ $$ = list2(MOD_EXPR,$1,$3); }
	| expr '+' expr
	{ $$ = list2(PLUS_EXPR,$1,$3); }
	| expr '-' expr
	{ $$ = list2(MINUS_EXPR,$1,$3); }
	| expr '*' expr
	{ $$ = list2(MUL_EXPR,$1,$3); }
	| expr '^' expr
	{ $$ = list2(BEKI_EXPR,$1,$3); }
	| expr RELOP expr
	{ yystype * tmp = (yystype *)(&($2));
	  $$ = list2(tmp->code,$1,$3); }

        | expr '?' expr ':' expr
	{ $$ = list3(TRY_EXPR,$1,$3,$5); }
	;

unary_expr:
	  primary_expr
	| '*' expr	%prec UNARY	/* pointer reference */ 
  	{ $$ = list1(POINTER_REF,$2); }
	| '-' expr	%prec UNARY	/* unary minus */
	{ $$ = list1(UNARY_MINUS_EXPR,$2); }
	;

primary_expr:
	 primary_expr '[' expr ']'
  	  { $$ = list2(ARRAY_REF,$1,$3); }
	| IDENTIFIER
	| CONSTANT
	| '('  expr  ')'
  	   { $$ = $2; }
	;

%%

#include "ninf_IDL_lex.c"




