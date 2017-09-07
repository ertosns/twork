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

\n {
  BEGIN(INITIAL);
}

"-help" return HELP;
"-exit" return EXIT;

"-st"|"-start" return START;
"-sp"|"-stop" return STOP;
"-l"|"-link" return LINK;
"-e"|"-event" return EVENT;

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

"-accum"|"-acc"|"-accumulate" return ACCUMULATE;

"--start-ssf" return SSF_START;
"--stop-ssf" return SSF_STOP;
"--list-ssf" return LIST_SSF_TASKS;
"--list-open-ssf" return LIST_OPEN_TASKS;

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
