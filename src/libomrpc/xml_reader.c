/* 
 * $Id: xml_reader.c,v 1.1.1.1 2004-11-03 21:01:19 yoshihiro Exp $
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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "xml_node.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define BUF_LEN 1024

static char buf[BUF_LEN];
static FILE *input;
static int have_peek_c = FALSE;
static int peek_c;
static int lineno = 1;

/* prototype */
static int get_char(void);
static void unget_char(int c);
static int skip_space(void);
static xml_node read_element(int c);
static xml_node read_element_extra(int c);
static xml_node read_attr(int c);
static void read_name(int c);
static void read_string(int c);

static char * xmalloc(int size);
static xml_node xml_new_node(enum xml_node_type type);
static xml_node_list xml_list_put_last(xml_node_list lx, xml_node x);

static void read_error(char *msg);

xml_node xml_read_document(FILE *fp)
{
    xml_node node,p;
    int c;

    input = fp;
    node = xml_new_node(XML_DOM);       /* DOM */
    while((c = skip_space()) != EOF){
        if(c != '<'){
            read_error("root document is missing");
        }
        p = read_element(get_char());
        if(p != NULL) node->child = xml_list_put_last(node->child,p);
    }

    if(c != EOF) read_error("illegal char at end of document");
    return node;
}

static int get_char()
{
    int c;
    if(have_peek_c){
        have_peek_c = FALSE;
        return peek_c;
    }
    c = getc(input);
    /* printf("[%c]",c); fflush(stdout); */
    if(c == '\n') lineno++;
    return c;
}

static void unget_char(int c)
{
    have_peek_c = TRUE;
    peek_c = c;
}

static int skip_space()
{
    int c;
    do {
        c = get_char();
    } while(isspace(c));
    return c;
}

/* called after reading '<' */
static xml_node read_element(int c)
{
    xml_node node,p;
    char *cp;

    if(c == '!' || c == '?'){ /* comment and CDATA */
        return read_element_extra(c);
    }
    node =  xml_new_node(XML_ELEMENT);
    read_name(c);
    node->name = strdup(buf);

next_attr:
    c = skip_space();
    switch(c){
    case EOF:
        read_error("unexpected EOF");
        return NULL;
    case '/':
        c = get_char();
        if(c == '>') {    /* end of elements */
            return node;
        } else read_error("'/' must be followed by '>'");
        return NULL;
    case '>':
        goto read_child;
    }
    p = read_attr(c);
    node->attrs = xml_list_put_last(node->attrs,p);
    goto next_attr;

read_child:
    c = get_char();
    if(c == '<') goto read_markup;
    cp = buf;
    do {
        *cp++ = c;
        if((cp-buf) >= BUF_LEN) read_error("too long text");
        c = get_char();
        if(c == EOF) read_error("unexpeced EOF");
    } while(c != '<');
    *cp = '\0';
    p = xml_new_node(XML_TEXT);
    p->value = strdup(buf);
    node->child = xml_list_put_last(node->child,p);

read_markup:
    c = get_char();
    if(c != '/'){
        p = read_element(c);
        if(p != NULL) node->child = xml_list_put_last(node->child,p);
        goto read_child;
    }
    /* </... >  */
    read_name(get_char());
    if(strcmp(node->name,buf) != 0){
        printf("%s %s\n",node->name,buf);
        read_error("un-matched '</'");
    }
    c = skip_space();
    if(c != '>')  read_error("'>' is expected");
    return node;
}

static xml_node read_attr(int c)
{
    xml_node node;

    node =  xml_new_node(XML_ATTR);
    read_name(c);
    node->name = strdup(buf);
    c = skip_space();
    if(c != '=') read_error("'=' is expected");
    c = skip_space();
    read_string(c);
    node->value = strdup(buf);

    return node;
}

static void read_name(int c)
{
    char *p;

    if(!isalpha(c) && c == '_') read_error("bad name");
    p = buf;
    do {
        *p++ = c;
        c = get_char();
    } while(isalpha(c) || isdigit(c)
            || c == '_' || c == ':' || c == '.');
    unget_char(c);
    *p = '\0';
}

static void read_string(int c)
{
    int end_c;
    char *p;

    if(c != '"' && c != '\'') read_error("quote string expected");
    end_c = c;
    p = buf;
    while((c = get_char()) != end_c){
        if(c == EOF) read_error("unexpcted EOF");
        *p++ = c;
    }
    *p ='\0';
}

/*
 * memory management
 */
#define XMALLOC(t,size)	((t)xmalloc(size))

/* allocate memory and initialize 0 */
static char   * xmalloc(int size)
{
    char   *p;
    p = (char *) malloc(size);
    if ( p == NULL )
        read_error("no memory");
    bzero(p, size);
    return (p);
}

static xml_node xml_new_node(enum xml_node_type type)
{
    xml_node p;

    p = XMALLOC(xml_node,sizeof(struct _xml_node));
    p->type = type;
    return p;
}

/* add expr 'x' as last node of list 'ex' */
static xml_node_list xml_list_put_last(xml_node_list lx, xml_node x)
{
    xml_node_list lp,lq;

    lp = XMALLOC(xml_node_list, sizeof(struct _xml_node_list));
    lp->next = NULL;
    lp->item = x;

    if (lx == NULL) return lp;
    for (lq = lx; lq->next != NULL ; lq = lq->next)/* */;
    lq->next = lp;
    return (lx);
}

void xml_node_print(xml_node x,FILE *fp)
{
    xml_node_list lp;

    switch(x->type){
    case XML_ELEMENT:
        fprintf(fp,"<%s",x->name);
        for(lp = x->attrs; lp != NULL; lp = lp->next)
            fprintf(fp," %s=\"%s\"",lp->item->name,lp->item->value);
        if(x->child == NULL){
            fprintf(fp,"/>");
            return;
        } else {
            fprintf(fp,">");
            for(lp = x->child; lp != NULL; lp = lp->next)
                xml_node_print(lp->item,fp);
            fprintf(fp,"</%s>",x->name);
        }
        break;
    case XML_TEXT:
        fprintf(fp,"%s",x->value);
        break;
    case XML_DOCTYPE:
        fprintf(fp,"<!DOCTYPE%s>",x->value);
        break;
    case XML_CDATA:
        fprintf(fp,"<[CDATA[%s]]>",x->value);
        break;
    case XML_PROLOG:
    case XML_PI:
        fprintf(fp,"<?%s?>",x->value);
        break;
    case XML_DOM:
        for(lp = x->child; lp != NULL; lp = lp->next){
            xml_node_print(lp->item,fp);
            fprintf(fp,"\n");
        }
        break;
    case XML_ATTR:
        fprintf(stderr,"unknown xml_node type=%d\n",x->type);
        exit(1);
    }
}

static void read_error(char *msg)
{
    fprintf(stderr,"xml read error (line %d):%s\n",lineno,msg);
    exit(1);
}

static xml_node read_element_extra(int c)
{
    int c1,c2;
    char *p;
    xml_node node;

    p = buf;
    if(c == '!'){
        c = get_char();
        if(c == '-'){
            c = get_char();
            if(c == '-'){
                /* start reading comment */
                while((c = get_char()) != EOF){
                    if(c == '-' && (c = get_char()) == '-'
                       && (c = get_char()) == '>')
                        return NULL;
                    if(c == EOF) break;
                }
            }
            goto error1;
        } else {
            c = get_char();
            if(c == '['){     /* <![CDATA ]]> */
                if((c = get_char()) == 'C' &&
                   (c = get_char()) == 'D' &&
                   (c = get_char()) == 'A' &&
                   (c = get_char()) == 'T' &&
                   (c = get_char()) == 'A' &&
                   (c = get_char()) == '['){
                    while((c = get_char()) != EOF){
                        if(c == ']'){
                            if((c1 = get_char()) == ']'){
                                if((c2 = get_char()) == '>'){
                                    /* make CDATA node */
                                    node =  xml_new_node(XML_CDATA);
                                    *p = '\0';
                                    node->value = strdup(buf);
                                    return node;
                                }
                                *p++ = c;
                                *p++ = c1;
                                *p++ = c2;
                            } else {
                                *p++ = c;
                                *p++ = c1;
                            }
                        } else *p++ = c;
                    }
                }
            } else if(c == 'D'){ /* <!DOCTYPE */
                if((c = get_char()) == 'O' &&
                   (c = get_char()) == 'C' &&
                   (c = get_char()) == 'T' &&
                   (c = get_char()) == 'Y' &&
                   (c = get_char()) == 'P' &&
                   (c = get_char()) == 'E'){
                    while((c = get_char()) != EOF){
                        if(c == '>'){
                            node =  xml_new_node(XML_DOCTYPE);
                            *p = '\0';
                            node->value = strdup(buf);
                            return node;
                        }
                        *p++ = c;
                    }
                }
            }
        }
    error1:
        if(c == EOF) read_error("unexpected EOF in <! markup");
        else read_error("<! is an illegal markup");
    } else if(c == '?'){ /* <?xml > */
        while((c = get_char()) != EOF){
            if(c == '?'){
                if((c1 = get_char()) == '>'){
                    if(strncmp("xml",buf,3) == 0)
                        node =  xml_new_node(XML_PROLOG);
                    else node = xml_new_node(XML_PI);
                    *p = '\0';
                    node->value = strdup(buf);
                    return node;
                } else {
                    *p++ = c;
                    *p++ = c1;
                }
            } else *p++ = c;
        }
        if(c == EOF) read_error("unexpected EOF in <? markup");
        else read_error("<? is an illegal markup");
    }
    read_error("error extra???");
    return NULL;
}

char *xml_attr_value(xml_node x,char *attr)
{
    xml_node_list lp;

    for(lp = x->attrs; lp != NULL; lp = lp->next)
        if(strcmp(lp->item->name,attr) == 0)
            return lp->item->value;
    return NULL;
}

