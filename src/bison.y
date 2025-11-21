%{

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <clogger.h>

#include "bison.h"
#include "lexer.h"
#include "parser.h"

int yyerror(YYLTYPE *yylloc, void *state, statement_list_t **statement_list, yyscan_t scanner, const char *msg);

%}

%code requires {
  #include "parser.h"
  typedef void* yyscan_t;
}

%output  "bison.c"
%defines "bison.h"

%define api.pure               /* pure (re‑entrant) parser */
%locations /* Turn on location support */

%lex-param   { yyscan_t scanner }          /* passed to yylex()   */
%parse-param { void *state }
%parse-param { statement_list_t **statement_list }  /* where we store the AST */
%parse-param { yyscan_t scanner }          /* we need it again   */

/* --------------------------------------------------------------------
   Value type declarations
   -------------------------------------------------------------------- */
%union {
    int          intvalue;          /* TOKEN_NUMBER */
    char        *charvalue;         /* TOKEN_STRING */

    statement_list_t *statement_list;
    statement_t *statement;
    parameter_t *parameter;

    struct {
        int count;
        parameter_t **params;
    } param_list_t;
}

%token TOKEN_ASSIGNMENT
%token TOKEN_LPAREN
%token TOKEN_RPAREN
%token TOKEN_PARAM_SEPARATOR
%token TOKEN_SEMICOLON
%token TOKEN_COLON
%token <intvalue> TOKEN_NUMBER  
%token <charvalue> TOKEN_STRING
%token TOKEN_INVALID

/* --------------------------------------------------------------------
   Non‑terminal type declarations
   -------------------------------------------------------------------- */
%type  <statement_list> programma
%type  <statement> label instruction
%type  <parameter> param
%type  <param_list_t> param_list

%start programma

%%

programma
    : 
        { 
            $$ = malloc(sizeof(statement_list_t));
            if (!$$) { perror("malloc"); exit(EXIT_FAILURE); }
            $$->count = 0;
            $$->statements = NULL;
            
            clog_info(__FILE_NAME__, "Anonymous");

            *statement_list = $$;
        }
    | programma label
        {
            /* $1 = statement_list_t* , $2 = statement_t* */
            if (!$1) {
                $$ = malloc(sizeof(statement_list_t));
                if (!$$) { perror("malloc"); exit(EXIT_FAILURE); }
                $$->count = 0;
                $$->statements = NULL;
            } else {
                $$ = $1;
            }
            $$->statements = realloc($$->statements,
                                     ($$->count + 1) * sizeof(statement_t*));
            if (!$$->statements) { perror("realloc"); exit(EXIT_FAILURE); }
            $$->statements[$$->count++] = $2;
            clog_info(__FILE_NAME__, "label: %d", $$->count);
            *statement_list = $$;
        }
    | programma instruction
        {
            /* same logic as for label */
            if (!$1) {
                $$ = malloc(sizeof(statement_list_t));
                if (!$$) { perror("malloc"); exit(EXIT_FAILURE); }
                $$->count = 0;
                $$->statements = NULL;
            } else {
                $$ = $1;
            }
            $$->statements = realloc($$->statements,
                                     ($$->count + 1) * sizeof(statement_t*));
            if (!$$->statements) { perror("realloc"); exit(EXIT_FAILURE); }
            $$->statements[$$->count++] = $2;
            clog_info(__FILE_NAME__, "instruction: %d", $$->count);
            *statement_list = $$;
        }
    | programma TOKEN_INVALID
        {
            /* report lexical error using the token location (@2) */
            yyerror(&@2, state, statement_list, scanner, "Unexpected character");
            $$ = $1; /* keep previous AST (no new statement) */
            clog_info(__FILE_NAME__, "Unexpected character: %d", $$->count);

            *statement_list = $$;
        }
    ;

label
    : TOKEN_STRING TOKEN_COLON
        { 
            $$ = parser_create_label($1); 
            free($1); 
        }
    ;

instruction
    : TOKEN_STRING TOKEN_SEMICOLON
        {
            $$ = parser_create_instruction(
                      $1,           
                      NULL,         
                      0);          
            free($1);
        }
    | TOKEN_STRING TOKEN_LPAREN param_list TOKEN_RPAREN TOKEN_SEMICOLON
        {
            /* $1  = function name (char*)
               $3  = arg vector (struct {SExpression **items; int count;})
            */
            $$ = parser_create_instruction(
                      $1,                
                      $3.params,
                      $3.count);
            free($1);
        }
    ;

param_list
    : param
        {
            $$.count = 1;
            $$.params = malloc(sizeof(parameter_t*));
            if(!$$.params) { perror("malloc"); exit(EXIT_FAILURE); }
            $$.params[0] = $1;
        }

    | param_list TOKEN_PARAM_SEPARATOR param
        {
            $$.count = $1.count + 1;
            $$.params = realloc($1.params,
                                 $$.count * sizeof(parameter_t*));
            if (!$$.params) { perror("realloc"); exit(EXIT_FAILURE); }
            $$.params[$1.count] = $3;
        }
    ;

/* -----------------------------------------------------------------
   PARAM – currently only a number, but you can extend it later.
   ----------------------------------------------------------------- */
param
    : TOKEN_NUMBER
        { $$ = parser_create_parameter_integer("default", $1); }
    | TOKEN_STRING
        { $$ = parser_create_parameter_string("default", $1); } 
    | TOKEN_STRING TOKEN_ASSIGNMENT TOKEN_NUMBER
        { $$ = parser_create_parameter_integer($1, $3); }
    | TOKEN_STRING TOKEN_ASSIGNMENT TOKEN_STRING
        { $$ = parser_create_parameter_string($1, $3); }
    ;

%%

int yyerror(YYLTYPE *yylloc, void *state, statement_list_t **statement_list, yyscan_t scanner, const char *msg)
{
    fprintf(stderr, "Syntax error on line %d:%d: %s\n",
            yylloc->first_line,yylloc->first_column, msg);
}