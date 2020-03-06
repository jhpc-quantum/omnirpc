static char rcsid[] = "$Id: omrpc_gen_mem.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $";
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
 *  $Id: omrpc_gen_mem.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $
 */
#include <stdlib.h>
#include <string.h>
#include "omrpc_gen.h"

/* C-front memory management */
/* no garbage collector, sorry. */
#define XMALLOC(t,size)	((t)xmalloc(size))

/*void *malloc(size_t size);*/

void
fatal(char *fmt, ...);

char * xmalloc(int size)
{
    char *p;
    if ((p = (char *)malloc(size)) == NULL)
      fatal("no memory");
    return(p);
}

#define SYMBOL_HASH_SIZE	0x400
#define SYMBOL_HASH_MASK	(SYMBOL_HASH_SIZE - 1)
SYMBOL symbol_hash_table[SYMBOL_HASH_SIZE];

SYMBOL find_symbol(char *name)
{
    SYMBOL sp;
    int hcode;
    char *cp;
    
    /* hash code, bad ?? */
    hcode = 0;
    for(cp = name; *cp != 0; cp++) hcode = (hcode <<1) + *cp;
    hcode &= SYMBOL_HASH_MASK;

    for(sp = symbol_hash_table[hcode]; sp != NULL; sp = sp->s_next)
      if(strcmp(name,sp->s_name) == 0) return(sp);

    /* not found, then allocate symbol */
    sp = XMALLOC(SYMBOL,sizeof(*sp));
    memset((char *)sp, 0, sizeof(*sp));
    sp->s_name = save_str(name);

    /* link it */
    sp->s_next = symbol_hash_table[hcode];
    symbol_hash_table[hcode] = sp;
    return(sp);
}

char *save_str(char *s)
{
    char *p;
    
    p = XMALLOC(char *,strlen(s)+1);
    strcpy(p,s);
    return(p);
}

double * save_float(float d)
{
    double *dp;
    
    dp = XMALLOC(double *,sizeof(double));
    *dp = d;
    return(dp);
}

expr make_enode(enum expr_code code, long int v)
{
    expr ep;
    
    ep = XMALLOC(expr,sizeof(*ep));
    ep->e_code = code;
    ep->e_lineno = lineno;
    ep->v.e_ival = v;
    return(ep);
}
expr make_enode_p(enum expr_code code, void * v)
{
    expr ep;
    
    ep = XMALLOC(expr,sizeof(*ep));
    ep->e_code = code;
    ep->e_lineno = lineno;
    ep->v.e_str = v;
    return(ep);
}

struct list_node *cons_list(expr x, struct list_node *l)
{
    struct list_node *lp;
    
    lp = XMALLOC(struct list_node *,sizeof(struct list_node));
    lp->l_next = l;
    lp->l_item = x;
    return(lp);
}

expr list0(enum expr_code code){
    return(make_enode_p(code,NULL));
}

expr list1(enum expr_code code, expr x1){
    return(make_enode_p(code,(void *)cons_list(x1,NULL)));
}

expr list2(enum expr_code code, expr x1, expr x2){
    return(make_enode_p(code,(void *)cons_list(x1,cons_list(x2,NULL))));
}

expr list3(enum expr_code code, expr x1, expr x2, expr x3){
    return(make_enode_p(code,(void *)cons_list(x1,cons_list(x2,cons_list(x3,NULL)))));
}

expr list4(enum expr_code code, expr x1, expr x2, expr x3, expr x4){
  return(make_enode_p(code,(void *)cons_list(x1,cons_list(x2,cons_list(x3,cons_list(x4,NULL))))));
}

expr list_put_last(expr lx, expr x){
  struct list_node *lp;
  
  lp = lx->v.e_lp;    
  if(lp == NULL)
    lx->v.e_lp = cons_list(x,NULL);
  else {
    for(; lp->l_next != NULL; lp = lp->l_next) /* */;
    lp->l_next = cons_list(x,NULL);
  }
  return(lx);
}
