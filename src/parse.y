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
  KeyVal *keyvalptr;
  Val *val;
  char *chars;
  int inttype;
  long double ldouble;
}

<<<<<<< HEAD
%token SQL EXIT NAME INTEGER DOUBLE
PASSCODE PATH HOURS MINUTES SECONDS
DEFINITION SLITERAL EOL START STOP EVENT UNDO
REDO CREATE ACCUMULATE INSERT UPDATE ALTER LINK
ADD_LINKABLE PRINT_COLUMNS DOES_TABLE_EXIST
VIEW_LAST_RECORDS REMOVE_LAST_RECORD DROP_TABLE GET_CURRENT_TASK

%type <chars> EXIT NAME SLITERAL PASSCODE
PATH DEFINITION  create insert update
viewlastrecords droptable removelastline
%type <keyvalptr> keyval
%type <val> VAL
%type <inttype> INTEGER exit start stop link TIME_TYPE
event undo redo accum linktable getcolumns doesexist getstate
=======
%token HELP EXIT NAME INTEGER DOUBLE
HOURS MINUTES SECONDS
START STOP EVENT UNDO REDO MUTE LIGHT
ACCUMULATE LINK
PRINT_COLUMNS DOES_TABLE_EXIST
VIEW_LAST_RECORDS REMOVE_LAST_RECORD DROP_TABLE
GET_CURRENT_TASK SSF_START SSF_STOP
LIST_SSF_TASKS LIST_OPEN_TASKS

%type <chars> EXIT NAME viewlastrecords droptable removelastline
%type <inttype> INTEGER exit start stop link TIME_TYPE event undo redo accum getcolumns doesexist gettask ssfstart ssfstop listssf listopenssf
>>>>>>> 9f01479... light alarm
%type <ldouble> DOUBLE NUMBER
%%

command: command command
| create
| insert
| update
| alter
| viewlastrecords
| removelastline
| droptable
| tablecommand
| general
;

tablecommand: start
| stop
| link
| event
| getstate
| accum
| undo
| redo
<<<<<<< HEAD
| linktable
| getcolumns
| doesexist
| alarm
=======
| mute
| light
| getcolumns
| doesexist
| alarm
| viewlastrecords
| removelastline
| droptable
| exit
>>>>>>> 9f01479... light alarm
;

general: exit
;


exit: EXIT
{
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
    int rc = startst($1);
    if(CUR_TASK)
        error(cat(3, "task ", $1, " opened"));
    if (!rc)
        error("start failed");
    else {
        CUR_TASK = $1;
        if (!hist) {
            free(hist);
            hist = NULL;
        }
    }
    cmdinit();
}
;

event: NAME DOUBLE EVENT
{
    if(!eventst($1, ftos($2)))
        error("event failed");
    else {
        if(!hist) {
            free(hist);
            hist = NULL;
        }
    }
    cmdinit();
}
|NAME INTEGER EVENT
{
    if(!eventst($1, itos($2)))
        error("event failed");
    else {
        if(!hist) {
            free(hist);
            hist = NULL;
        }
    }
    cmdinit();
}
;

getstate: GET_CURRENT_TASK
{
    printf("current task: %s\n", CUR_TASK);
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

redo: NAME REDO
{
  if(!redost($1, hist))
    error("redo failed");
  cmdinit();
}
;

TIME_TYPE: MINUTES { $$ = MINUTES; };
| HOURS { $$ = HOURS; };
| SECONDS { $$ = SECONDS; };
;

alarm: NAME NUMBER TIME_TYPE
{
<<<<<<< HEAD
  int type = linkabletype($1);
  if(!type)
    error("givin table isn't accumulatable!");
  // fix
  else if(type != start)
    error("givin table isn't alarmable!");
  else {
    int period;
    switch ($3) {
    case SECONDS: period = $2; break;
    case MINUTES: period = $2*60; break;
    case HOURS: period = $2*60*60; break;
=======
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
      //TODO add condition for new devices.
      alert($1, period, LIGHTS_LIGHT);
>>>>>>> 9f01479... light alarm
    }
    alert($1, period);
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

create: SQL NAME keyval CREATE
{
  if(!strcmp($2, DAILY_TERMINATED) ||
     !strcmp($2, LINKABLES)) {
    printf("givin table name is forbidden\n");
  }
  else {
    exec(createcmd, $3, $2, NULL);
    cmdinit();
  }
}
;

insert: SQL NAME keyval INSERT
{
  if(!strcmp($2, DAILY_TERMINATED) ||
     !strcmp($2, LINKABLES)) {
    printf("givin table name is forbidden\n");
  }
  else {
    exec(insertcmd, $3, $2, NULL);
    cmdinit();
  }
}
;

update: SQL NAME keyval UPDATE {
    if(!strcmp($2, DAILY_TERMINATED) ||
       !strcmp($2, LINKABLES)) {
        printf("givin table name is forbidden\n");
    }
    else {
        exec(updatecmd, $3, $2, NULL);
        cmdinit();
    }
}
| SQL NAME keyval NAME UPDATE {
  if(!strcmp($2, DAILY_TERMINATED) ||
     !strcmp($2, LINKABLES)) {
    printf("givin table name is forbidden\n");
  }
  else {
    exec(updatecmd, $3, $2, $4);
    cmdinit();
  }
}
;

alter: SQL NAME NAME DEFINITION ALTER {
  if(!strcmp($2, DAILY_TERMINATED) ||
     !strcmp($2, LINKABLES)) {
    printf("givin table name is forbidden\n");
  }
  else {
    String names[] = {$2, $3};
    sqlAlter(names, NULL, def_sdt_type);
    cmdinit();
  }
 }
| SQL NAME NAME DEFINITION VAL VAL ALTER
{
  if(!strcmp($2, DAILY_TERMINATED) ||
     !strcmp($2, LINKABLES)) {
    printf("givin table name is forbidden\n");
  }
  else {
    String names[] = {$2, $3};
    if($5->valtype==$6->valtype==sdt_type) {
      String ref[] = {$5->strep, $6->strep};
      sqlAlter(names, ref, def_sdt_type);
    } else printf("wrong foreign key\n");
    cmdinit();
  }
}
;

linktable: NAME ADD_LINKABLE
{
  if(!strcmp($1, DAILY_TERMINATED) ||
     !strcmp($1, LINKABLES)) {
    printf("givin table name is forbidden\n");
  }
  else {
    if(!addlinkable($1))
      error("add linkable failed");
    cmdinit();
  }
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

viewlastrecords: NAME INTEGER NAME VIEW_LAST_RECORDS
{
  if(validpasscode($3)) {
    String clause = cat(5, "ROWID > ((SELECT MAX(ROWID) FROM ", $1, ") - ", itos($2), ")");
    Result *res = sqlRead($1, NULL, 0, 0, 0, clause);
    handleRes(res);
    viewTable();
    free(res);
  }
  else printf("unvalid passcode");
  cmdinit();
}
;

removelastline: NAME NAME REMOVE_LAST_RECORD
{
  if(validpasscode($2)) {
    deleteLastRow($1);
  }
  else printf("givin passcode isn't valid");
  cmdinit();
}
;

droptable: NAME NAME DROP_TABLE
{
  if(validpasscode($2)) {
    deleteTable($1);
    if(!strcmp($1, LINKABLES)) {
      deleteTable(DAILY_TERMINATED);
    }

    printf("table removed\n");
  }
  else printf("passcode isn't valid");
  cmdinit();
}
;

keyval:  keyval keyval { $$ = addkeyval($1, $2); }
| NAME VAL { $$ = makekeyval($1, $2); }
;

VAL: NAME { $$ = makevalptr($1, sdt_type); }
| SLITERAL { $$ = makevalptr($1, sdt_string); }
| INTEGER { $$ = makevalptr(itos($1), sdt_number); }
| DOUBLE { $$ = makevalptr(ftos($1), sdt_double); }
| PATH { $$ = makevalptr($1, sdt_blob); }
| DEFINITION { $$ = makevalptr($1, def_sdt_type); }
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
    initcrud();
    initalarm();
    initloc();
    initatrack();
    /*if (getenv("TWORK_DEV")) {
        struct sigaction *act = malloc(sizeof(struct sigaction));
        act->sa_sigaction = catch;
        act->sa_flags = SA_SIGINFO;
        sigaction(SIGSEGV, act, NULL);
        }*/
}

int main(int argc, char **argv)
{
    init();
    cmdinit();
    if(argc > 1)
        if(!strcmp(argv[1],  "-d"))
            dbgmode = 1;
        else dbgmode = 0;

    return yyparse();
}
