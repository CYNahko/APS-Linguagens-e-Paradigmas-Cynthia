#include <stdio.h>
#include "parser.tab.h"
extern int yylex(void);
extern char *yytext;
int main(void){
  int t;
  while ((t=yylex())) {
    printf("TOK %d '%s'\n", t, yytext?yytext:"");
    if (t==0) break;
  }
  return 0;
}

