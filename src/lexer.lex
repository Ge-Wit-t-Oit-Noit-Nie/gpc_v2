%option outfile="lexer.c" header-file="lexer.h"
%option warn nodefault

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge
%option bison-locations
%option yylineno

%{
#include "bison.h"          /* brings in token definitions and yylval */
#include "Expression.h"
#include <string.h>
#include <stdlib.h>
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno;

%}

/* --------------------------------------------------------------
   Macro definitions – these are expanded *before* the rule section.
   -------------------------------------------------------------- */
DIGIT       [0-9]
LETTER      [A-Za-z_]
NUMBER      {DIGIT}+
STRING      {LETTER}+

/* ------------------------------------------------------------------ */

%%

{NUMBER} {
                yylval->intvalue = atoi(yytext);
                return TOKEN_NUMBER;
            }

{STRING} {
                yylval->charvalue = strdup(yytext);
                return TOKEN_STRING;
            }

"("        { return TOKEN_LPAREN; }
")"        { return TOKEN_RPAREN; }
","        { return TOKEN_PARAM_SEPARATOR; }
";"        { return TOKEN_SEMICOLON; }
":"        { return TOKEN_COLON; }

[ \t\r]+   { /* ignore whitespace */ }

"\n"      { ++yylineno;}

"." {
        fprintf(stderr, "Unexpected character '%c' (0x%02x) at line %d\n",
                yytext[0] ? yytext[0] : '?', (unsigned char)yytext[0], yylineno);
        return TOKEN_INVALID;
    }

%%