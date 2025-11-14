#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "clogger.h"

#include "Expression.h"
#include "bison.h"
#include "lexer.h"

int yyparse(void *, SExpression **expression, yyscan_t scanner);

SExpression *getAST(const char *expr)
{
    SExpression *expression;
    yyscan_t scanner;
    YY_BUFFER_STATE state;
    YYLTYPE yylval;

    if (yylex_init(&scanner))
    {
        /* could not initialize */
        return NULL;
    }

    state = yy_scan_string(expr, scanner);

    if (yyparse(&yylval, &expression, scanner))
    {
        /* error parsing */
        return NULL;
    }

    yy_delete_buffer(state, scanner);

    yylex_destroy(scanner);

    return expression;
}
int evaluate(SExpression *e)
{
    switch (e->type)
    {
    case eVALUE:
        return e->value;
    case eMULTIPLY:
        return evaluate(e->left) * evaluate(e->right);
    case eADD:
        return evaluate(e->left) + evaluate(e->right);
    default:
        /* should not be here */
        return 0;
    }
}

int main(int argc, char *argv[]) {

    char test[] = {"START:\nwait(10);STOP;"};

    SExpression *e = getAST(test);
    if (e == NULL) {
        return EXIT_FAILURE;
    }
    //int result = evaluate(e);
    //printf("Result of '%s' is %d\n", test, result);
    switch (e->type)
    {
    case eLABEL:
        printf("Parsed label for '%s'\n", e->string);

        break;
    case eINSTRUCTION:
        printf("Parsed instruction for '%s'\n", e->string);
        break;

    default:
        printf("Unknown type '%s'\n", e->string);

        break;
    }

    deleteExpression(e);

    return EXIT_SUCCESS;
}   