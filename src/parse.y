%{
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <signal.h>
#ifndef ALARM
#include "src/alarm.h"
#endif
#ifndef LINKER
#include "src/linker.h"
#endif
#ifndef LOC_HDR
#include "src/loc.h"
#endif
#ifndef ATRACK
#include "src/atrack.h"
#endif
  
  void yyerror(const char *);
  void cmdinit();
  void nothing();
  int rule; //what is the current rule being parsing
  int def_sdt_type; //givin sdt_type for current keyval pair
  Hist *hist;
  String historytable = NULL;
  extern int isfresh(String);
  siginfo_t *siginfo;
%}

%define parse.error verbose
%union {
  Val *val;
  char *chars;
  int inttype;
  long double ldouble;
}

%token HELP EXIT NAME INTEGER DOUBLE
HOURS MINUTES SECONDS
START STOP EVENT UNDO REDO
ACCUMULATE LINK
PRINT_COLUMNS DOES_TABLE_EXIST
VIEW_LAST_RECORDS REMOVE_LAST_RECORD DROP_TABLE
GET_CURRENT_TASK SSF_START SSF_STOP
LIST_SSF_TASKS LIST_OPEN_TASKS

%type <chars> EXIT NAME viewlastrecords droptable removelastline
%type <inttype> INTEGER exit start stop link TIME_TYPE event undo redo accum getcolumns doesexist gettask ssfstart ssfstop listssf listopenssf
%type <ldouble> DOUBLE NUMBER
%%

command: command command
| help
| start
| stop
| event
| link
| ssfstart
| ssfstop
| listssf
| listopenssf
| gettask
| accum
| undo
| redo
| getcolumns
| doesexist
| alarm
| viewlastrecords
| removelastline
| droptable
| exit
{ cmdinit(); };

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
     
   TABLE -l|link
        link workd of day, associate all start-stop, event class tables with occurance date, add worke of day see below.";
  printf("%s\n", help);
  */
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

NUMBER: INTEGER { $$ = (double)$1; }
| DOUBLE { $$ = $1; }
;

stop: NAME STOP
{
  
  if(!stopst($1))
    error("stop failed");
  else {
    if(!hist) {
      free(hist);
      hist = NULL;
    }
  }
  CUR_TASK = NULL;
  cmdinit();
}
;

start: NAME START
{
  //TODO prevent multitasking, also with event
  //change viewing {5,6,0} -> {start, stop, event}
  String opened;
  if ( (opened = read_cur_task()) )
    error(cat(3, "failed! other task ", opened, " running"));
  else {
    if (!startst($1))
      error("start failed");
    else {
      CUR_TASK = strdup($1);
      if (!hist) {
	free(hist);
	hist = NULL;
      }
    }
  }
  cmdinit();
}
;

event: NAME DOUBLE EVENT
{

  if ( read_cur_task() )
    error(cat(3, "failed! task ", $1, " opened"));
  else {
    if(!eventst($1, ftos($2)))
      error("event failed");
    else {
      if(!hist) {
	free(hist);
	hist = NULL;
      }
    }
  }
  cmdinit();
}
|NAME INTEGER EVENT
{
  if ( read_cur_task() )
    error(cat(3, "failed! task ", $1, " opened"));
  else {
    if(!eventst($1, itos($2)))
      error("event failed");
    else {
      if(!hist) {
	free(hist);
	hist = NULL;
      }
    }
  }
  cmdinit();
}
;

ssfstart: NAME SSF_START
{
    ssf_start($1);
    cmdinit();
}
;

ssfstop: NAME SSF_STOP
{
    ssf_stop($1);
    cmdinit();
}
;

listssf: LIST_SSF_TASKS
{
  String *list = list_ssf_tasks();
  if (list) {
    for (int i = 0; list[i]; i++)
      printf("(%d) %s\n",i, list[i]);
    free(list);
  }
  cmdinit();
}
;

listopenssf: LIST_OPEN_TASKS
{
  String *list = list_current_ssf_tasks();
  if (list) {
    for (int i = 0; list[i]; i++)
      printf("(%d) %s\n",i, list[i]);
    free(list);
  }
  cmdinit();
}
;

gettask: GET_CURRENT_TASK
{
  printf("current task: %s\n", read_cur_task());
  cmdinit();
}
;

accum: NAME ACCUMULATE
{
    if(notexist($1))
        printf("givin table doesn't exist\n");
    if (isfresh($1)) {
        State *st = accumulate($1);
        if(st)
            printf("accumulated %LF of %s today\n keep it up!" ,st->val, $1);
        else
            printf("no data yet!\n");
    }
    else
        printf("table isn't fresh\n");
    cmdinit();
}
;

undo: NAME UNDO
{
  if(!historytable || strcmp($1, historytable)) {
    if(!hist) {
      free(hist);
      hist = NULL;
    }
    historytable = $1;
  }
  if(!hist)
      hist = calloc(1, sizeof(Hist));
  if(!undost($1, hist))
    error("undo failed");

  if (CUR_TASK) {
    free(CUR_TASK);
    CUR_TASK = NULL;
  }
  cmdinit();
}
;

redo: NAME REDO
{
  if(!redost($1, hist))
    error("redo failed");
  else {
    if (CUR_TASK)
      free(CUR_TASK);
    CUR_TASK = strdup($1);
  }
  cmdinit();
}
;

TIME_TYPE: MINUTES { $$ = MINUTES; };
| HOURS { $$ = HOURS; };
| SECONDS { $$ = SECONDS; };
;

alarm: NAME NUMBER TIME_TYPE
{
  State *ls = last_state($1);
  if (ls) {
    if(ls->type != stop)
      error("conflict action");
    else {
      int period;
      switch ($3) {
      case SECONDS: period = $2; break;
      case MINUTES: period = $2*60; break;
      case HOURS: period = $2*60*60; break;
      }
      alert($1, period);
    }
    free(ls);
  }
  cmdinit();
}
;

link: LINK
{
  if(!link_accumulatables())
    error("link failed");
  else {
    if(!hist) {
      free(hist);
      hist = NULL;
    }
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

%%

void yyerror (const char *s) {
  error(cat(9,"error!\n", (char *)s, "\n\nparser content:\n",
            "  string:", yylval.chars,
            "\n  int:", itos(yylval.inttype),
            "\n  long double:", ftos(yylval.ldouble)));
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
  initlinker();
  initutils();
  initcrud();
  initalarm();
  initloc();
  //initatrack(); //use zeitgeist instead
  String *tasks = list_current_ssf_tasks();
  int cnt = 0;
  if (tasks) {
    highlight("open ssf tasks");
    while (tasks[cnt]) {
      printf("(%d) %s\n", cnt, tasks[cnt]);
      free(tasks[cnt]);
      cnt++;
    }
  }
}

int main(int argc, char **argv)
{
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
