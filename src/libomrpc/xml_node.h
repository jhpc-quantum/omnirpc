/* 
 * $Id: xml_node.h,v 1.1.1.1 2004-11-03 21:01:19 yoshihiro Exp $
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
 * quick-hack simple XML DOM (Document Object Model) structure
 */
#include <stdio.h>

enum xml_node_type {
    XML_ELEMENT = 0,	/* default */
    XML_ATTR,
    XML_TEXT,
    XML_CDATA,
    XML_PROLOG,
    XML_PI,
    XML_DOCTYPE,
    XML_DOM
};

/* de-syntax program is represented by this data structure. */
typedef struct _xml_node {
    enum xml_node_type type;
    char *name;		/* tagname or attribute name */
    char *value;
    struct _xml_node_list *attrs;
    struct _xml_node_list *child;
} * xml_node;

/* list data structure, which is ended with NULL */
typedef struct _xml_node_list {
    struct _xml_node_list *next;
    xml_node item;
} *xml_node_list;

#define LIST_NEXT(lp)	((lp)->next)
#define LIST_ITEM(lp)	((lp)->item)

#define IS_XML_ELEMENT(x,s) ((x)->type == XML_ELEMENT && strcmp((x)->name,s) == 0)

xml_node xml_read_document(FILE *fp);
void xml_node_print(xml_node x,FILE *fp);
char *xml_attr_value(xml_node x,char *attr);


