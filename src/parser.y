%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void emit(const char *fmt, ...);

extern int yylex(void);
extern char *yytext;
void yyerror(const char *s);

%}

%token T_SRRl T_HRRASH T_VLETH T_DRAZH T_ESSHL T_FRRAL T_SHRELK T_ZRRAN T_SRRYL T_ARROW
%token T_IDENTIFIER T_NUMBER T_STRING
%left '+' '-'
%left '*' '/'

%union {
  long num;
  char *str;
}

%type <str> T_IDENTIFIER T_STRING
%type <num> T_NUMBER
%type <str> IDENTIFIER

%%

PROGRAM:
    /* empty */           { /* nothing */ }
  | PROGRAM STATEMENT     { /* allow multiple statements */ }
  ;

STATEMENT:
    PROCESS_DECL          { /* process declared */ }
  | LINK_STMT
  | MUTATE_STMT
  | MERGE_STMT
  | IF_STMT
  | LOOP_STMT
  | RETURN_STMT
  ;

PROCESS_DECL:
    T_SRRl IDENTIFIER '(' PARAM_LIST_OPT ')' '{' PROCESS_BODY '}' 
      {
        emit("LABEL %s", $2);
        emit("LABEL %s_end", $2);
      }
  ;

PARAM_LIST_OPT:
    /* empty */           { /* no params (ignore for now) */ }
  | PARAM_LIST
  ;

PARAM_LIST:
    IDENTIFIER            { /* can map params to registers later */ }
  | PARAM_LIST ',' IDENTIFIER
  ;

PROCESS_BODY:
    /* empty */           { }
  | PROCESS_BODY STATEMENT
  ;

LINK_STMT:
    T_HRRASH IDENTIFIER T_ARROW IDENTIFIER ';'
      {
        /* mapping: Hrrash a -> b  => PARTILHAR a,b (assembly) */
        emit("PARTILHAR %s, %s", $2, $4);
      }
  ;

MUTATE_STMT:
    T_VLETH IDENTIFIER EXPRESSION ';'
      {
        /* mapping: Vleth id expr  -> set id = expr then MUTAR id */
        /* simple: evaluate expr into literal (only constants support in proto) */
        emit("// MUTATE %s ... expr below", $2);
        emit("MUTAR %s", $2);
      }
  ;

MERGE_STMT:
    T_DRAZH IDENTIFIER ',' IDENTIFIER T_ESSHL IDENTIFIER ';'
      {
        /* Drazh a, b Esshl c  -> MERGE a and b into c  => emit as CONDENSE and REGISTER */
        emit("CONDENSAR %s, %s -> %s", $2, $4, $6);
      }
  ;

IF_STMT:
    T_FRRAL '(' EXPRESSION ')' '{' PROCESS_BODY '}' IF_ELSE_OPT
      {
        /* simple mapping: create unique labels */
        static int _ifcount = 0;
        int id = _ifcount++;
        emit("// IF start");
        /* expression must set flags — in prototype we assume CMP emitted */
        emit("IFGT if_true_%d", id);
        emit("JMP if_false_%d", id);
        emit("LABEL if_true_%d", id);
        /* body already emitted */
        emit("JMP if_end_%d", id);
        emit("LABEL if_false_%d", id);
        /* else body emitted by IF_ELSE_OPT if present */
        emit("LABEL if_end_%d", id);
      }
  ;

IF_ELSE_OPT:
    /* empty */           { }
  | T_SHRELK '{' PROCESS_BODY '}'
  ;

LOOP_STMT:
    T_ZRRAN '(' EXPRESSION ')' '{' PROCESS_BODY '}'
      {
        static int _loopcount = 0;
        int id = _loopcount++;
        emit("LABEL loop_start_%d", id);
        /* assume EXPRESSION emits comparison and flags */
        emit("IFZERO loop_end_%d", id);
        /* body emitted */
        emit("JMP loop_start_%d", id);
        emit("LABEL loop_end_%d", id);
      }
  ;

RETURN_STMT:
    T_SRRYL EXPRESSION ';'
      {
        emit("RETURN");
      }
  ;

/* EXPRESSION: keep it simple for prototype — support numbers, ids, binary ops */
EXPRESSION:
    T_NUMBER              { emit("/* push num %ld */", $1); }
  | IDENTIFIER            { emit("/* ref id %s */", $1); }
  | '(' EXPRESSION ')'    { /* already emitted */ }
  | EXPRESSION '+' EXPRESSION { emit("/* add */"); }
  | EXPRESSION '-' EXPRESSION { emit("/* sub */"); }
  | EXPRESSION '*' EXPRESSION { emit("/* mul */"); }
  | EXPRESSION '/' EXPRESSION { emit("/* div */"); }
  ;

IDENTIFIER:
    T_IDENTIFIER          { $$ = $1; }
  ;

%%

/* simple emitter */
#include <stdarg.h>
void emit(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
}

void yyerror(const char *s) {
  fprintf(stderr, "Parse error: %s\n", s);
}
