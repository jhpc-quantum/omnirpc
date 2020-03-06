static char rcsid[] = "$Id: ninf_IDL_lex.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $";
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
 *  lexical analyzer
 *
 *  $Id: ninf_IDL_lex.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $
 */

/**  function declarations  **/

extern expr make_enode(enum expr_code code, long int i);
extern expr make_enode_p(enum expr_code code, void * p);

extern void error(char *fmt, ...);
extern void fatal(char *fmt, ...);

static int read_identifier(int ch);
static int read_number(int ch);
static int read_string_constant(int mark);


/* buffer size for lexical analyzer */
#define LEX_BUFSIZE 0x8000	/* 4K */

static char yytext[LEX_BUFSIZE];

extern struct keyword_entry 
{
    char *kw_name;
    enum symbol_type kw_type;
    int kw_value;
} keyword_table[];

expr error_expr;

/* initialize parser */
void
initialize_lex()
{
  struct keyword_entry *kp;
  SYMBOL sp;
  
  /* intern all keywords */
  for(kp = keyword_table; kp->kw_name != NULL; kp++){
    sp = find_symbol(kp->kw_name);
    sp->s_type = kp->kw_type;
    sp->s_value = kp->kw_value;
  }

  /* make constant */
  error_expr = make_enode(ERROR_NODE,0);
}

#define MAX_LINE_LEN	1024
static char line_buf[MAX_LINE_LEN];
static char *linep = NULL;
static int have_peekc = FALSE;
static int line_peekc;
int lineno = 0;

#define GETCH()		lex_getc()
#define UNGETCH(c)	(have_peekc++, line_peekc = (c))

int lex_getc()
{
  extern char title_file_name[];
  char *cp;
  
  if (have_peekc){
    have_peekc = FALSE;
    return(line_peekc);
  }
  if (linep == NULL || *linep == '\0'){
    next_line:
    if (fgets(line_buf,MAX_LINE_LEN,source_file) == NULL)
      return(EOF);
    linep = line_buf;
    lineno++;
    
    if (debug_flag)  fprintf(stderr, "%3d:%s",lineno,line_buf);
    
#ifdef not
    /* check line nubmer, # nnn "file" */
    if (line_buf[0] == '#'){
      linep = &line_buf[1];
      while (*linep == ' ') linep++;	/* skip space */
      lineno = 0;
      while (isdigit((int)(*linep)))
	lineno = lineno*10 + *linep++ - '0';
      while (*linep == ' ') linep++;	/* skip space */
      if (*linep == '"'){  /* parse file name */
	linep++;
	cp = title_file_name;
	while(*linep != '"' && *linep != '\n')
	  *cp++ = *linep++;
	*cp = '\0';
      }
      goto next_line;
    }
#endif
  }
  return(*linep++);
}

/* VARARGS */
static int yylast;

/* error printing routine in parser */
void
yyerror(char *s) 
{ 
  if(yylast < 0x7F)
    error("%s at or near '%c'",s,yylast);
  else
    switch(yylast){
    default:
      error(s);
      break;
    case MODE:
      error("%s at keyword '%s'",s,yytext);
      break;
    case TYPE:
      error("%s at type keyword '%s'",s,yytext);
      break;
    case IDENTIFIER:
      error("%s at symbol '%s'",s,yytext);
      break;
    case CONSTANT:
      error("%s at constant '%s'",s,yytext);
      break;
    }
}

int
yylex0();

/* lexical analyer */
int
yylex()
{
    yylast = yylex0();
    return(yylast);
}

int
yylex0()
{
  int ch;
  
  for (;;){
    switch (ch = GETCH()){
      /* white space */
    case ' ':	case '\t':
    case '\b':  case '\r': case '\f':
      continue;
      
    case '\n':
      continue;
      
    case EOF:
    case '(':  case ')':
    case '{':  case '}':
    case '[':  case ']':
    case '*':  case '%':
    case '?':  case ':':  case ';': 
    case '^':  case '~':  case ',':
      /* single charactor */
      return(ch);
      
    case '|':	/* | and || */
#ifdef not
      if((ch = GETCH()) == '|')
	return(OROR);
      UNGETCH(ch);
#endif
      return('|');
      
    case '&':	/* & and && */
#ifdef not
      if((ch = GETCH()) == '&')
	return(ANDAND);
      UNGETCH(ch);
#endif
      return('&');
      
    case '<':	/* < and << */
      switch (ch = GETCH()){
      case '<':
	return(LSHIFT);
      case '=':
	yylval.code = LE_EXPR;
	return(RELOP);
      }
      UNGETCH(ch);
      yylval.code = LT_EXPR;
      return(RELOP);
      
    case '>':	/* > and >> and >= */
      switch(ch = GETCH()){
      case '>':
	return(RSHIFT);
      case '=':
	yylval.code = GE_EXPR;
	return(RELOP);
      }
      UNGETCH(ch);
      yylval.code = GT_EXPR;
      return(RELOP);
      
    case '!':	/* ! and != */
      if((ch = GETCH()) == '='){
	yylval.code = NEQ_EXPR;
	return(RELOP);
      }
      UNGETCH(ch);
      return('!');
      
    case '=':	/* = and == */
      if((ch = GETCH()) == '='){
	yylval.code = EQ_EXPR;
	return(RELOP);
      }
      UNGETCH(ch);
      return('=');

    case '+':  /* + and ++ */
#ifdef not
      if((ch = GETCH()) == '+')
	return(PLUSPLUS);
      UNGETCH(ch);
#endif
      return('+');
      
    case '-':	/* - and -- and -> */
#ifdef not
      switch(ch = GETCH()){
      case '-':
	return(MINUSMINUS);
      case '>':
	return(STREF);
      }
      UNGETCH(ch);
#endif
      return('-');
      
    case '/':
      if((ch = GETCH()) == '*') {
	/* scan comment */
	for (;;){
	  ch = GETCH();
	  if (ch == '*' && (ch = GETCH()) == '/') break;
	  if (ch == EOF){
	    error("unexpected EOF");
	    return(EOF);
	  }
	}
	continue;
      }
      UNGETCH(ch);
      return('/');
      
    case '.':
      ch = GETCH();
      UNGETCH(ch);	/* peek */
      if(ch >= '0' && ch <= '9') return(read_number('.'));
      return('.');
      
    case '0': case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7': case '8': case '9': 
      return(read_number(ch));
      
    case '"':
      return(read_string_constant('"'));
      
    case '\'':
      return(read_string_constant('\''));
      
    default:
      /* identifier */
      if (!isalpha(ch)){
	error("illegal character: 0x% in hex", ch);
	break;
      }
    case '_':
    case '$':
      return(read_identifier(ch));
    }
  }
}

/* collect an identifier, check for reserved word, and return */
int read_identifier(int ch)  	/* first char */
{
  char *cp;
  SYMBOL sp;
  
  cp = yytext;
  do {
    *cp++ = ch;
    ch = GETCH();
    if(cp >= &yytext[LEX_BUFSIZE-1])
      {
	fatal("too long identifier");
	break;
      }
  } while(isalnum(ch) || ch == '_' || ch == '$');
    UNGETCH(ch);	/* push back */
  *cp = '\0';
  sp = find_symbol(yytext);
  
  switch (sp->s_type){
  case S_KEYWORD:
    return(sp->s_value);
  case S_TYPE:
    yylval.val = make_enode(BASIC_TYPE_NODE,sp->s_value);
    return(TYPE);
  case S_CLASS:
    yylval.val = make_enode(MODE_SPEC_NODE,sp->s_value);
    return(MODE);
  case S_IDENT:
    yylval.val = make_enode_p(IDENT,sp);
    return(IDENTIFIER);
  default:
    fatal("read_identifier");
  }
  return -INT_MAX;
}

int
read_number(int ch)  /* first char */
{
  char *cp;
  long int value;
  
  value = 0;
  if(ch == '0'){
    ch = GETCH();
    if(ch == 'x' || ch == 'X'){
      /* HEX */
      for(;;){
	ch = GETCH();
	if(!isxdigit(ch)) break;
	if(isdigit(ch)) 
	  value = value * 16 + ch - '0';
	else if(isupper(ch))
	  value = value * 16 + ch - 'A' + 10;
	else 
	  value = value * 16 + ch - 'a' + 10;
      }
    }
    if(ch == '.') goto read_floating;
    else  { /* octal */
      while(ch >= '0' && ch <= '7'){
	value = value * 8 + ch - '0';
	ch = GETCH();
      }
    }
    goto ret_INT;
  }
  /* else decimal or floating */
  read_floating:
  cp = yytext;
  while(isdigit(ch)){
    value = value * 10 + ch - '0';
    *cp++ = ch;
    ch = GETCH();
  }
  if(ch != '.' && ch != 'e' && ch != 'E') goto ret_INT;
  /* floating */
  if(ch == '.') {
    *cp++ = ch;
    /* reading floating */
    ch = GETCH();
    while(isdigit(ch)){
      *cp++ = ch;
      ch = GETCH();
    }
  }
  if(ch == 'e' || ch == 'E'){
    *cp++ = 'e';
    ch = GETCH();
    if(ch == '+' || ch == '-'){
      *cp++ = ch;
      ch = GETCH();
    }
    while(isdigit(ch)){
      *cp++ = ch;
      ch = GETCH();
    }
  }
  UNGETCH(ch);
  *cp = '\0';
  yylval.val = make_enode_p(FLOAT_CONSTANT,save_float(atof(yytext)));
  return(CONSTANT);
  
ret_INT:
  if(ch == 'L')
    yylval.val = make_enode(LONG_CONSTANT,value);
  else {
    UNGETCH(ch);
    yylval.val = make_enode(INT_CONSTANT,value);
  }
  return(CONSTANT);
}

/* simple version */
int
read_string_constant(int mark)
{
  int ch;
  char *cp;
  int value,i;
  
  cp = yytext;
  while ((ch=GETCH()) != mark) {
    switch (ch) {
    case EOF:
      error("unexpected EOF");
      break;
      
#ifdef not
    case '\n':
      error("newline in string or char constant");
      break;
#endif
    case '\\':	/* escape */
      switch (ch = GETCH()){
      case '\n':
	continue;
      case 'n':
	ch = '\n';
	break;
      case 'r':
	ch = '\r';
	break;
      case 'b':
	ch = '\b';
	break;
      case 't':
	ch = '\t';
	break;
      case 'f':
	ch = '\f';
	break;
      case 'v':
	ch = '\013';
	break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
	value = ch -'0';
	ch = GETCH();  /* try for 2 */
	if(ch >= '0' && ch <= '7')
	  {
	    value = (value << 3) | (ch - '0');
	    ch = GETCH();
	    if(ch >= '0' && ch <= '7')
	      value = (value << 3) | (ch - '0');
	    else UNGETCH(ch);
	  }
	else UNGETCH(ch);
	ch = value;
	break;
      }
    default:
      *cp++ = ch;
    }
    if (cp >= &yytext[LEX_BUFSIZE-1]){
      fatal("too long string");
      break;
    }
  }
  *cp = '\0';
  
  /* end of string or  char constant */
  if (mark == '"'){
    yylval.val = make_enode_p(STRING_CONSTANT,save_str(yytext));
    return(STRING);
  } else { 
    /* end the character constant */
    if(cp == yytext) error("empty character constant");
    if((cp - yytext) > (sizeof(int)/sizeof(char)))
      error("too many characters in character constant");
    value = 0;
    for (i = 0; i < sizeof(int); i++){
      if(yytext[i] == 0) break;
      value = (value << 8)| (0xFF & yytext[i]);
    }
    yylval.val = make_enode(INT_CONSTANT,value);
  }
  return(CONSTANT);
}

expr read_rest_of_body(int flag)
{
  char *cp;
  char ch, in_string;
  int nest_level;
  
  cp = yytext;
  if(flag) *cp++ = '{';	/* already read */
  in_string = 0;
  nest_level = 1;
  do {
    ch = GETCH();
    if(ch == EOF) {
      error("unexpected EOF");
      break;
    }
    else if(ch == '\\') {
      /* escape */
      *cp++ = ch;
      *cp++ = GETCH();
      continue;
    }
    if(in_string != 0 && in_string == ch)
      in_string = 0;
    else if(in_string == 0){
      /* out string */
      if(ch == '"'|| ch == '\'')
	in_string = ch;
      /* else count nest level */
      else if(ch == '{') nest_level++;
      else if(ch == '}') nest_level--;
    }
    *cp++ = ch;
  } while(nest_level > 0);
  if(!flag) cp--;
  *cp = '\0';
  return(make_enode_p(STRING_CONSTANT,save_str(yytext)));
}


struct keyword_entry keyword_table[]=
{
    {"mode_in",		S_CLASS,	(int)MODE_IN},
    {"mode_out",	S_CLASS,	(int)MODE_OUT},
    {"mode_inout",	S_CLASS,	(int)MODE_INOUT},
    {"mode_work",	S_CLASS,	(int)MODE_WORK},
    {"IN",	S_CLASS,	(int)MODE_IN},
    {"OUT",	S_CLASS,	(int)MODE_OUT},
    {"INOUT",	S_CLASS,	(int)MODE_INOUT},
    {"WORK",	S_CLASS,	(int)MODE_WORK},

    {"char",	S_TYPE,		(int)DT_CHAR},
    {"short",	S_TYPE,		(int)DT_SHORT},
    {"int",	S_TYPE,		(int)DT_INT},
    {"long",	S_TYPE,		(int)DT_LONG},
    {"longlong",	S_TYPE,		(int)DT_LONGLONG},
    {"float",	S_TYPE,		(int)DT_FLOAT},
    {"double",	S_TYPE,		(int)DT_DOUBLE},
    {"unsigned",S_TYPE,		(int)DT_UNSIGNED},
    {"string",  S_TYPE,		(int)DT_STRING_TYPE},
    {"scomplex",	S_TYPE,		(int)DT_SCOMPLEX},
    {"dcomplex",	S_TYPE,		(int)DT_DCOMPLEX},
    {"void",	S_TYPE,		(int)DT_VOID},
    {"filename",	S_TYPE,		(int)DT_FILENAME},    
    {"filepointer",	S_TYPE,		(int)DT_FILEPOINTER},    

    {"Module",	S_KEYWORD,	MODULE},
    {"CompileOptions",	S_KEYWORD,	COMPILE_OPTIONS},
    {"Globals",S_KEYWORD,	GLOBALS},
    {"Define",	S_KEYWORD,	DEFINE},
    {"Calls",	S_KEYWORD,	CALLS},
    {"Backend",	S_KEYWORD,      BACKEND},
    {"Shrink",	S_KEYWORD,      SHRINK},
    {"Required",	S_KEYWORD,	REQUIRED},
    {"Language",	S_KEYWORD,	LANGUAGE},
    {"CalcOrder",	S_KEYWORD,	CALCORDER},
    {"Library",	S_KEYWORD,	LIBRARY},
    {"FortranFormat",	S_KEYWORD,	FORTRANFORMAT},
    /* {"CALLBACK", S_KEYWORD,	CALLBACK}, */
    {NULL,0,0}
};

