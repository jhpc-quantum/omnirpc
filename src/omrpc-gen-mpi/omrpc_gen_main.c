static char rcsid[] = "$Id: omrpc_gen_main.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $";
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
/* 
 *  OmniRPC stub generator
 */
#include <string.h>
#include <stdlib.h>
#include "omrpc_gen.h"

#define FILE_NAME_LEN 	125
char title_file_name[FILE_NAME_LEN];

/* for debug */
int debug_flag = FALSE;
FILE *debug_fp;

/* the number of errors */
int nerrors;

char *source_file_name,*output_file_name;
FILE *source_file,*output_file;

char *program;	/* this program */

/**  function declarations  **/

extern int yyparse(void);

static void check_nerrors(void);
static void print_usage(void);

/*
 *  MAIN
 */

int main(int argc, char *argv[])
{
    register char *cp;

    program = argv[0];

    --argc;
    ++argv;

    /* parse command line */
    while (argc >0 && argv[0][0] == '-') {
      for (cp = argv[0]+1; *cp; cp++){
	switch (*cp){
	case 'd':
	  ++debug_flag;
	  break;
	case 'h':
	  print_usage();
	  exit(0);
	default:
	  break;
	}
      }
      --argc;
      ++argv;
    }
    
    if (argc > 0){
	source_file_name = argv[0];
      if ((source_file = fopen(source_file_name, "r")) == NULL){
	  fprintf(stderr,"cannot open %s\n",source_file_name);
	  exit(1);
      }
      /* set title as default */
      strcpy(title_file_name,source_file_name);
    } else 
      source_file = stdin;
    
    if (argc > 1){
      output_file_name = argv[1];
      if ((output_file = fopen(output_file_name,"w")) == NULL){
	fprintf(stderr,"cannot open %s\n",output_file_name);
	exit(1);
      }
    } else 
      output_file = stdout;

    /* DEBUG */ debug_fp = stderr;

    init_expr_code_info();
    initialize_lex();
    
    /* start processing */
    yyparse();

    if(source_file != stdin) fclose(source_file);

    if (debug_flag) dump_stubs();

    if (!nerrors){
	generate_rpc_executable(output_file);
    }
    if(output_file != stdout) fclose(output_file);

    return(nerrors?1:0);
}

/* 
 * error messages 
 */

void where(int lineno)
{ 
    /* print location of error  */
    fprintf(stderr, "\"%s\", line %d: ",title_file_name, lineno);
}

/* nonfatal error message */
/* VARARGS0 */
void error(char *fmt, ...) 
{ 
    va_list args;

    ++nerrors;
    where(lineno); /*, "Error");*/
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n" );
    fflush(stderr);
    check_nerrors();
}

/* VARARGS0 */
void error_at_node(expr x, char *fmt, ...) 
{ 
    va_list args;
    ++nerrors;
    va_start(args, fmt);
    where((int)EXPR_LINENO(x)); /* , "ErrorAtNode"); */
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n" );
    fflush(stderr);
    if(nerrors > 30){
      /* give the compiler the benefit of the doubt */
      fprintf(stderr, 
	      "too many error, cannot recover from earlier errors: goodbye!\n" );
      exit(1);
    }
}

/* VARARGS0 */
void warning_at_node(expr x, char *fmt, ...) 
{ 
  va_list args;

  where(EXPR_LINENO(x)); /*, "WarnAtNode"); */
  fprintf(stderr,"warning:");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n" );
  fflush(stderr);
}

void
check_nerrors()
{
    if(nerrors > 30){
      /* give the compiler the benefit of the doubt */
      fprintf(stderr, 
	      "too many error, cannot recover from earlier errors: goodbye!\n" );
      exit(1);
    }
}

/* compiler error: die */
/* VARARGS1 */
void
fatal(char *fmt, ...)
{ 
    va_list args;
    
    where(lineno); /*, "Fatal");*/
    fprintf(stderr, "compiler error: " );
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n" );
    fflush(stderr);
    abort();
}

int warning_flag = FALSE; 

/* warning */
void
warning(char *fmt, ...) 
{  
  va_list args;
  
  if(warning_flag) return;
  where(lineno); /*, "Warn");*/
  va_start(args, fmt);
  fprintf(stderr, "warning: " );
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n" );
  fflush(stderr);
}

/*
 *   Print usage
 */

void
print_usage(void)
{
    puts("usage: omrpc-gen [-d] [-h] [input_file] [output_file]");
    puts("\t-d  Debug option");
    puts("\t-h  Show this message");
}

/* EOF */




