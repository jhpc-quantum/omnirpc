#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

typedef union
{
    expr val;
    enum expr_code code;
} yystype;
#define YYSTYPE yystype

#endif
