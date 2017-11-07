%option noyywrap nodefault yylineno caseless

Name            [a-zA-Z]{1,30}
digit           [0-9]

%{
#include <stdio.h>
#include <stdlib.h>
#ifndef DEBUG
#include "src/debug.h"
#endif
#ifndef UTILS
#include "src/utils.h"
#endif
#include "parse.tab.h"

extern void cmdinit();
%}

%%

[\t\v" "] ;

"\n" return EOL;

"-help" return HELP;
"-exit" return EXIT;

"-start"  return START;
"-stop"   return STOP;
"-event"  return EVENT;

"-task" return GET_CURRENT_TASK;
"-undo" return UNDO;
"-redo" return REDO;

"-mute" return MUTE;
"-light" return LIGHT;

"-col"|"-columns" return PRINT_COLUMNS;
"-exist" return DOES_TABLE_EXIST;
"-view" return VIEW_LAST_RECORDS;

"-r"|"-rlr"|"--remove-last-record" return REMOVE_LAST_RECORD;
"-d"|"-drop" return DROP_TABLE;


"--start-branch" return BRANCH_START;
"--break-branch" return BRANCH_BREAK;
"--break-leaf"   return BREAK_LEAF;
"--start-piv" return PIV_START;
"-pivot" return PIVOT;
"--del-piv" return PIV_DEL;
"--list-pivs" return LIST_PIVS;
"--list-open-pivs" return LIST_OPEN_PIVS;

"sec"|"s"|"seconds"|"second" return SECONDS;
"min"|"minute"|"minutes"|"m" return MINUTES;
"hours"|"hour"|"hr"|"hrs"|"h" return HOURS;


{Name} {
  yylval.chars = strdup(yytext);
  return NAME;
}

{digit}+ {
  yylval.inttype = atoi(strdup(yytext));
  return INTEGER;
}

{digit}+\.{digit}* {
  //TODO free copied str
  yylval.ldouble = strtold(strdup(yytext), NULL);
  return DOUBLE;
}

. {
  printf("ambigious command!\nwrite twork -help\n");
}
%%
