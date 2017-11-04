%{

#include <ctype.h>
#include <signal.h>
  
#ifndef ALARM
#include "src/alarm.h"
#endif
  
#ifndef LINKER
#include "src/linker.h"
#endif
  
#ifndef LOCHEADER
#include "src/loc.h"
#endif
  
#ifndef ATRACK
#include "src/atrack.h"
#endif
  
#ifndef CLOCK
#include "src/clock.h"
#endif
  
#ifndef PIVOTHEADER
#include "src/pivot.h"
#endif
  
#ifndef SSF_H
#include "src/ssf.h"
#endif
  
  void yyerror(const char *);
  void cmdinit();
  void nothing();
  int rule; //what is the current rule being parsing
  int def_sdt_type; //givin sdt_type for current keyval pair
  siginfo_t *siginfo;
  %}

%define parse.error verbose
%union {
  Val *val;
  char *chars;
  int inttype;
  long double ldouble;
}

%token HELP EXIT NAME INTEGER DOUBLE START STOP EVENT
HOURS MINUTES SECONDS  UNDO REDO MUTE LIGHT
PRINT_COLUMNS DOES_TABLE_EXIST
VIEW_LAST_RECORDS REMOVE_LAST_RECORD DROP_TABLE
GET_CURRENT_TASK PIV_START PIV_DEL
LIST_PIVS LIST_OPEN_PIVS BRANCH_BREAK BRANCH_START BREAK_LEAF PIVOT EOL

%type <chars> EXIT NAME viewlastrecords droptable removelastline

%type <inttype> INTEGER
exit startcmd stopcmd  eventcmd undo redo getcolumns alarm
doesexist gettask pivstart pivdel listpivs listopenpivs branchstart branchbreak breakleaf pivot

%type <ldouble> DOUBLE NUMBER SEC
%%

command: command command
| command EOL
| help
| startcmd
| stopcmd
| eventcmd
| branchstart
| branchbreak
| breakleaf
| pivstart
| pivot
| pivdel
| listpivs
| listopenpivs
| gettask
| undo
| redo
| mute
| light
| getcolumns
| doesexist
| alarm
| viewlastrecords
| removelastline
| droptable
| exit
;

help: HELP
{
  /*

  String hlp = " twork based on accumulatable tables with name TABLE, \
and actions {start, stop, event}. \

   tasks uses {start, stop, value(event)}, only one tasks could be active at a time.\
     TABLE -st|-start|start
       push start action if given name exists, or create new accumulatable with 
       given name, and push start action. 
     TABLE -sp|-stop
       push stop action iff table with given name exists
     TABLE FLOAT
       event action, descriped by given float value i.e 7, or 3.14
       pushed to TABLE_NAME of class EVENT.
      start-stop class accepts only event stop iff start pushed first.
     TABLE -task
        return current task name

   undowing events:
     TABLE -undo
       undo event
     TABLE -redo
       redo last undoed events stacked in FIFO list, *only before linking.
     TABLE -dl


   table information:
     TABLE -col
       list given table col names seperated by '|' character
     TABLE -exist
       check if given table name exists
     TABLE INTEGER -view
       list last given INTEGER row values.
     TABLE -r|-rlr|--remove-last-record
     TABLE -d|-drop
       remove table
     TABLE -accum
   

   SSF \"self-taught syllabus flow\" spans long-term periods {days..years} and multiple tasks,   using accumulatable tables, several SSF's could be opened at a time, i.e on vacation, and learning language.
     TABLE --start-ssf
        same as start for tasks
     TABLE --stop-ssf
        same as stop for tasks
     TABLE --list-ssf
        list all availible SSF activities
     TABLE --list-open-ssf
        list current SSF's
   
   alarms:
     TABLE TIME[WHILE SPACE]SCALE
        push start on given start-stop TABLE, and alert after TIME  period according to SCALE {sec|s|seconds|second} for seconds, 
        {min|minute|minutes|m} for mintues, {hours|hour|hr|hrs|h} for hours.
        alert does using choosing alarming mean as described below
  */
}
;

NUMBER: INTEGER { $$ = (double)$1; }
| DOUBLE { $$ = $1; }
;

SEC: NUMBER SECONDS { $$ = $1; };
| NUMBER MINUTES    { $$ = $1*60; };
| NUMBER HOURS      { $$ = $1*60*60; };
;

startcmd: NAME START
{
  stopst(read_cur_task());
  if (!startst($1))
    error("start failed");  
  cmdinit();
}
| NAME START SEC
{
  stopst(read_cur_task());
  if (!latestate($1, NULL, start, $3))
    error("start failed");
  //TODO check history, and that blow repetitive snippet.}
  cmdinit();
}
;

stopcmd: NAME STOP
{  
  if(!stopst($1))
    error("stop failed");   
  cmdinit();
}
| NAME STOP SEC
{
  if (!latestate($1, NULL, stop, $3))
    error("start failed");
  cmdinit();
}
;

eventcmd: NAME NUMBER EVENT
{
  if ( read_cur_task() )
    error(cat(3, "failed! task ", $1, " opened"));
  
  else {
    if(!eventst($1, ftos((double)$2)))
      error("event failed");
  }
  cmdinit();
}
|NAME NUMBER EVENT SEC
{
  if (!latestate($1, ftos((double)$2), event, (long long) $4))
    error("start failed");
  cmdinit();
}
;

branchstart: NAME NAME BRANCH_START
{
  add_branch($1, $2);
  cmdinit();
}
;

branchbreak: NAME NAME BRANCH_BREAK
{
  break_branch($1, $2);
  cmdinit();
}
;


pivstart: NAME PIV_START
{
  start_pivot($1);
  cmdinit();
}
;


breakleaf: NAME BREAK_LEAF
{
  break_leaf($1);
  cmdinit(); 
}
;

pivot: NAME PIVOT
{
  Pivot *piv = get_pivot($1);
  if (piv) {
    String pivstr = cumulate_pivot_str(piv);
    if (piv) {
      highlight(pivstr);
      free(pivstr);
    }
    freepiv(piv);
  }
  cmdinit();
}
;

pivdel: NAME PIV_DEL
{
    del_pivot($1);
    cmdinit();
}
;

listpivs: LIST_PIVS
{
int size;
  String *list = list_pivots(&size);
  for (int i = 0; i<size; i++)
    printf("(%d) %s\n",i, list[i]);
  des_strs(list, size);
  cmdinit();
}
;

listopenpivs: LIST_OPEN_PIVS
{
  int size;
  String *list = list_current_pivots(&size);
  for (int i = 0; i<size; i++)
    printf("(%d) %s\n",i, list[i]);
  des_strs(list, size);
  cmdinit();
}
;

gettask: GET_CURRENT_TASK
{
  printf("current task: %s\n", read_cur_task());
  cmdinit();
}
;

undo: NAME UNDO
{
  if(!undost($1))
    error("undo failed");
  cmdinit();
}
;

redo: NAME REDO
{
  if(!redost($1))
    error("redo failed");
  cmdinit();
}
;

mute: MUTE
{
  mute_lights();
  cmdinit();
}
;

light: LIGHT
{
  lights_flag(LIGHTS_LIGHT);
  cmdinit();
}
;

alarm: NAME SEC
{
  if (validstate($1, start)) {
    alert($1, $2, LIGHTS_LIGHT);
  }
  cmdinit();
}
;

getcolumns: NAME PRINT_COLUMNS
{
  int size;
  String *columns = acolumns($1, &size);
  for(int i = 0; i < size; i++)
    printf("%s|",columns[i]);
  printf("\n");
  free(columns);
  //TODO does each element should be freed alone?
  cmdinit();
}
;

doesexist: NAME DOES_TABLE_EXIST
{
  if(notexist($1))
    printf("NO\n");
  else printf("YES\n");
  cmdinit();
}
;

viewlastrecords: NAME INTEGER VIEW_LAST_RECORDS
{
  String clause = cat(5, "ROWID > ((SELECT MAX(ROWID) FROM ", $1, ") - ", itos($2), ")");
  Result *res = sqlRead($1, NULL, 0, 0, 0, clause);
  if (res->type == tableres) {
    viewTable();
    free(res);
  }
  cmdinit();
}
|NAME VIEW_LAST_RECORDS
{
  Result *res = sqlReadFull($1);
  if (res->type == tableres) {
    viewTable();
    free(res);
  }
  cmdinit();
}
;

removelastline: NAME REMOVE_LAST_RECORD
{
  deleteLastRow($1);
  cmdinit();
}
;

droptable: NAME DROP_TABLE
{
  removelinkable($1);
  cmdinit();
}
;

exit: EXIT
{
  finalizecrud();
  printf("exiting...\n");
  exit(0);
  $$ = 0;
}
;

%%

void yyerror (const char *s) {
   error(cat(9,"error!\n", (char *)s, "\n\nparser content:\n",
            "string:", yylval.chars,
            "\nint:", itos(yylval.inttype),
            "\nlong double:", ftos(yylval.ldouble)));
  
  highlight("unvalid command!");
}

void cmdinit() {
  printf("\ntwork> ");
}

void catch(int sig, siginfo_t *info, void *ignore) {
    //catch signal for debugger.
    error("segmentation fault!\n");
    siginfo=info;
}

void init() {
  assert(initlinker());
  assert(initutils());
  assert(initcrud());
  assert(initalarm());
  assert(initclock());
  assert(initloc());
  //initatrack(); //use zeitgeist instead
  int size;
  String *pivs = list_current_pivots(&size);
  if (size>0)
    highlight("open pivots");
  for (int i = 0; i < size; i++)
      printf("(%d) %s\n", i, pivs[i]);
  if (size>0)
    des_strs(pivs, size);
}

int main(int argc, char **argv)
{
  //TODO change word limit {colName}
  if(argc > 1) {
    if (!strcmp(argv[1],  "-d")) {
      dbgmode = 1;
      highlight("debug mood started!");
    }
    else dbgmode = 0;
  }
  init();
  cmdinit();
  return yyparse();
}
