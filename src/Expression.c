/*
 * Expression.c
 * Implementation of functions used to build the syntax tree.
 */

#include "Expression.h"

#include <stdlib.h>

/**
 * @brief Allocates space for expression
 * @return The expression or NULL if not enough memory
 */
static SExpression *allocateExpression()
{
    SExpression *b = (SExpression *)malloc(sizeof(SExpression));

    if (b == NULL)
        return NULL;

    b->type = eVALUE;
    b->value = 0;

    b->left = NULL;
    b->right = NULL;

    b->string = (char *)malloc(256 * sizeof(char));

    return b;
}
SExpression *createLabel(char *name)
{
    SExpression *b = allocateExpression();

    if (b == NULL)
        return NULL;

    b->type = eLABEL;
    strcpy(b->string, name);

    return b;
}
SExpression *createInstruction(char *name)
{
    SExpression *b = allocateExpression();

    if (b == NULL)
        return NULL;

    b->type = eINSTRUCTION;
    strcpy(b->string, name);

    return b;
}
/* In Expression.h or wherever you keep your AST constructors */
SExpression *createInstructionWithParam(const char *name,
                                        SExpression **args,
                                        int argc) {
    SExpression *b = allocateExpression();

    if (b == NULL)
        return NULL;

    b->type = eINSTRUCTION;
    strcpy(b->string, name);
    if(argc > 0)
        b->left = args[0]; // assuming only one argument for simplicity
    return b;
}
SExpression *createNumber(int value)
{
    SExpression *b = allocateExpression();

    if (b == NULL)
        return NULL;

    b->type = eVALUE;
    b->value = value;

    return b;
}

SExpression *createOperation(EOperationType type, SExpression *left, SExpression *right)
{
    SExpression *b = allocateExpression();

    if (b == NULL)
        return NULL;

    b->type = type;
    b->left = left;
    b->right = right;

    return b;
}

void deleteExpression(SExpression *b)
{
    if (b == NULL)
        return;

    deleteExpression(b->left);
    deleteExpression(b->right);

    free(b);
}