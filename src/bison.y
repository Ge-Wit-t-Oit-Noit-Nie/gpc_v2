%{

/*
 * Parser.y file
 * To generate the parser run: "bison Parser.y"
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "Expression.h"
#include "bison.h"

int yyerror(YYLTYPE *yylloc, void *state, SExpression **expression, yyscan_t scanner, const char *msg);

%}

%code requires {
  #include "Expression.h"
  typedef void* yyscan_t;
}

%output  "bison.c"
%defines "bison.h"

%define api.pure               /* pure (re‑entrant) parser */
%locations /* Turn on location support */

%lex-param   { yyscan_t scanner }          /* passed to yylex()   */
%parse-param { void *state }
%parse-param { SExpression **expression }  /* where we store the AST */
%parse-param { yyscan_t scanner }          /* we need it again   */

/* --------------------------------------------------------------------
   Value type declarations
   -------------------------------------------------------------------- */
%union {
    int          intvalue;        /* TOKEN_NUMBER */
    char        *charvalue;    /* TOKEN_STRING */
    SExpression **expressions;         /* any non‑terminal */

    SExpression *expression;         /* any non‑terminal */
    struct {
        SExpression **items;   /* dynamic array of arguments */
        int          count;    /* how many items */
    } argvec;                  /* used for arg_list */
}


%token TOKEN_LPAREN             "("
%token TOKEN_RPAREN             ")"
%token TOKEN_PARAM_SEPARATOR    ","
%token TOKEN_SEMICOLON          ";"
%token TOKEN_COLON              ":"
%token <intvalue> TOKEN_NUMBER  "[0-9]+"
%token <charvalue> TOKEN_STRING "[A-Za-z]+"
%token TOKEN_INVALID

/* --------------------------------------------------------------------
   Non‑terminal type declarations
   -------------------------------------------------------------------- */
%type  <expression> programma label instruction param
%type  <argvec>  arg_list

%start programma

%%

programma
    : 
        { *expression = NULL; }
    | programma label
        { *expression = $2; }
    | programma instruction
        { *expression = $2; }
    ;

label
    : TOKEN_STRING TOKEN_COLON
        { $$ = createLabel($1); free($1); *expression = $$; }        
    ;

instruction
    : TOKEN_STRING TOKEN_SEMICOLON
        {
            $$ = createInstructionWithParam(
                      $1,           
                      NULL,         
                      0);          
            free($1);
        }
    | TOKEN_STRING TOKEN_LPAREN arg_list TOKEN_RPAREN TOKEN_SEMICOLON
        {
            /* $1  = function name (char*)
               $3  = arg vector (struct {SExpression **items; int count;})
            */
            $$ = createInstructionWithParam(
                      $1,                 /* name */
                      $3.items,           /* array of argument expressions */
                      $3.count);          /* how many arguments */
            free($1);                 /* we allocated a copy in the lexer */
            /* clean up the temporary vector – the 
            arguments themselves
               belong to the AST and must NOT be freed here. */
            free($3.items);
        }
    ;

arg_list
    : param
        {
            $$ .count = 1;
            $$ .items = malloc(sizeof(SExpression*));
            if (!$$ .items) { perror("malloc"); exit(EXIT_FAILURE); }
            $$ .items[0] = $1;          /* $1 is the SExpression* from param */
        }

    | arg_list TOKEN_PARAM_SEPARATOR param
        {
            $$ .count = $1 .count + 1;
            $$ .items = realloc($1 .items,
                                 $$ .count * sizeof(SExpression*));
            if (!$$ .items) { perror("realloc"); exit(EXIT_FAILURE); }
            $$ .items[$1 .count] = $3;   /* $3 is the new param */
        }
    ;

/* -----------------------------------------------------------------
   PARAM – currently only a number, but you can extend it later.
   ----------------------------------------------------------------- */
param
    : TOKEN_NUMBER
        { $$ = createNumber($1); }
    ;

%%

int yyerror(YYLTYPE *yylloc, void *state, SExpression **expression, yyscan_t scanner, const char *msg)
{
    fprintf(stderr, "Syntax error on line %d:%d: %s\n",
            yylloc->first_line,yylloc->first_column, msg);
}