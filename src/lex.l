%option noyywrap nodefault yylineno caseless

Name            [a-zA-Z]{1,30}
Literal         \"[^\"]*\"
digit           [0-9]
Path            (\/{Name})+

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

"-h" printf("help\n");

[(),|\t\v=" "] ;

\n {
  BEGIN(INITIAL);
}

"-exit" return EXIT;

"-st"|"-start" return START;
"-sp"|"-stop" return STOP;
"-l"|"-link" return LINK;
"-e"|"-event" return EVENT;
"-task" return GET_CURRENT_TASK;
"-undo" return UNDO;
"-redo" return REDO;
"-dl"|"--add-linkable" return ADD_LINKABLE;
"-c"|"-create" return CREATE;
"-i"|"-insert" return INSERT;
"-u"|"-update" return UPDATE;
"-col"|"-columns" return PRINT_COLUMNS;
"-exist" return DOES_TABLE_EXIST;
"-view" return VIEW_LAST_RECORDS;
"-r"|"-rlr"|"--remove-last-record" return REMOVE_LAST_RECORD;
"-d"|"-drop" return DROP_TABLE;
"-sql" return SQL;
"-alter" return ALTER;
"-accum"|"-acc"|"-accumulate" return ACCUMULATE;

"sec"|"s"|"seconds"|"second" return SECONDS;
"min"|"minute"|"minutes"|"m" return MINUTES;
"hours"|"hour"|"hr"|"hrs"|"h" return HOURS;


"b"|"blob"|"blb" {
  def_sdt_type=sdt_blob;
  yylval.chars = strdup(yytext);
  return DEFINITION;
}
"t"|"type" {
  def_sdt_type=sdt_type;
  yylval.chars = strdup(yytext);
  return DEFINITION;
}
"n"|"number"|"integer"|"int"|"float"|"double" {
  def_sdt_type=sdt_number;
  yylval.chars = strdup(yytext);
  return DEFINITION;
}
"d"|"date"|"timestamp"|"datetime"|"time" {
  def_sdt_type=sdt_date;
  yylval.chars = strdup(yytext);
  return DEFINITION;
}
"s"|"literal"|"text"|"varchar" {
  def_sdt_type=sdt_string;
  yylval.chars = strdup(yytext);
  return DEFINITION;
}

{Name} {
  yylval.chars = strdup(yytext);
  return NAME;
}

{Path} {
  yylval.chars = strdup(yytext);
  return PATH;
}

{Literal} {
  String tmp =
     calloc(strlen(yytext)-2, sizeof(String));
  for(int i = 1; i < strlen(yytext)-1; i++)
      tmp[i-1] = yytext[i];
      yylval.chars = tmp;
  return SLITERAL;
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

%%
