%option noyywrap
%option yylineno

%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"
extern int yylineno;
%}

/* Definitions (macros) */
CONSONANT   [srlhvdzkmg]
VOWEL       [aeiou]
SYLL       {CONSONANT}{VOWEL}{CONSONANT}?
IDENT      ({SYLL})+

DIGIT       [0-9]
NUMBER      {DIGIT}+
STRING      \"([^"\\]|\\.)*\"

%%

[ \t\r\n]+               { /* ignora espaços em branco */ }
"//".*                   { /* comentário de linha */ }
"#".*                    { /* comentário (compatível com MicroSong) */ }

"Srrl"                   { return T_SRRl; }
"Hrrash"                 { return T_HRRASH; }
"Vleth"                  { return T_VLETH; }
"Drazh"                  { return T_DRAZH; }
"Esshl"                  { return T_ESSHL; }
"Frral"                  { return T_FRRAL; }
"Shrelk"                 { return T_SHRELK; }
"Zrran"                  { return T_ZRRAN; }
"Srryl"                  { return T_SRRYL; }

"->"                     { return T_ARROW; }
"("                      { return '('; }
")"                      { return ')'; }
"{"                      { return '{'; }
"}"                      { return '}'; }
","                      { return ','; }
";"                      { return ';'; }
"+"                      { return '+'; }
"-"                      { return '-'; }
"*"                      { return '*'; }
"/"                      { return '/'; }

{IDENT}                  {
                            yylval.str = strdup(yytext);
                            return T_IDENTIFIER;
                         }

{NUMBER}                 {
                            yylval.num = atoll(yytext);
                            return T_NUMBER;
                         }

{STRING}                 {
                            int L = strlen(yytext);
                            char *s = (char*)malloc(L-1);
                            if(!s) exit(1);
                            strncpy(s, yytext+1, L-2);
                            s[L-2] = '\0';
                            yylval.str = s;
                            return T_STRING;
                         }

.                        { return yytext[0]; }

%%
