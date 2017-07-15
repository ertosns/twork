#include <string.h>
#include <time.h>
#include "crud.h"
#include "clock.h"
#ifndef CRUD
#include "crud.h"
#endif
#ifndef LINKER
#include "linker.h"
#endif
#define LOCHEADER

//todo fix state start->stop, event
//todo accumulation table chagned

const int accumulatable = 1;
const int naccumulatable = 0;
String STATE = "STATE";
String VAL = "VAL";
String DATE = "DATE"; //default column
String SSF = "SSF";
String SSF_TASK = "SSF_TASK";
Hist *addhist(Hist *prev, State *cur) {
  Hist *history = malloc(sizeof(Hist));
  Hist *histprev;
  if(prev) {
        histprev = malloc(sizeof(Hist));
        memcpy(histprev, prev, sizeof(Hist));
  } else histprev = NULL;
  history->previous = histprev;
  history->current = cur;
  history->size = (prev==NULL)?1:prev->size+1;
  prev = history;
  return history;
}

extern Hist *hist;
void pophist(Hist *history) {
    hist = history->previous;
}

void createTclk(String ctable) {
    Val stateval[] = { makeval(VAL, sdt_double),
                       makeval(STATE, sdt_number)};
    sqlCreate(ctable, stateval, 2);
}

int insertts(String ctable, String dval, int state) {
    Val vals[] = { makeval((dval)?dval:"0", sdt_double),
                   makeval(itos(state), sdt_number) };
    String names[] = {ctable, VAL, STATE};
    return sqlInsert(names, vals, 2);
}

/*valid states
  none-->start
  start->stop
  stop-->push
  stop-->start
  push-->start
  start->undo
  stop-->undo

  none-->event
  event->event
  event->push
  push->event
  event->undo
*/

//TODOS passing static ptr for initialization gets overwritten

String strstate(int state) {
    switch(state) {
    case event: return "event";
    case start: return "start";
    case stop: return "stop";
    case push: return "push";
    case none: return "none";
    case undo: return "undo";
    }
    return NULL;
}

int validstate(String ctable, int st) {
  State *laststate = state(ctable);
  switch(laststate->type) {
  case none:
    if (!(st == start || st == event)) {
      error(cat(2, "empty table expecting start|event but gets ", strstate(st)));
      return 0;
    }
    return 1;
  case start:
    if (!(st == stop || st == undo)) {
      error(cat(2, "start state expecting stop|undo but gets ", strstate(st)));
      return 0;
    }
    return 1;
  case stop:
    if (!(st == undo || st == push || st == start)) {
      error(cat(2, "stop state expecting undo|push|start but gets ", strstate(st)));
      return 0;
    }
    return 1;
  case push:
    if (!(st == start || st == event)) {
      error(cat(2, "push expecting start|event but gets ", strstate(st)));
      return 0;
    }
    return 1;
  case event:
    if(!(st == event || st == push || st == undo)) {
      error(cat(2, "event expecting event|push|undo but gets ", strstate(st)));
      return 0;
    }
    return 1;
  default:
    error("unkown state!");
    return FAILED;
  }
  //TODO (res)  does freeing laststate free laststate->date?
  free(laststate->date);
  free(laststate);
  return SUCCESS;
}

void tableres2states(Table *table, State *states) {
  Row *rw = table->row;
  for(int r = 0; r < table->size && rw; r++) {
    states[r].rowid = atoi(rw->val[0]);
    states[r].date = malloc(sizeof(struct tm));
    strptime(rw->val[1], SQL_DATE_FORMAT, states[r].date);
    states[r].val = strtold(rw->val[2], NULL);
    states[r].type = atoi(rw->val[3]);
    rw = rw->nxt;
  }
}

//TODO (fix) warning -> rm extern, why gcc isn't smart enough?
extern int linkablexists(String);

State *accumulate (String table) {
  
  if (!linkablexists(table)) {
    printf("not accumulated table");
    return NULL;
  }
  
  String getpushid = cat(4, prependType(NAME_COL, sdt_type) , " = '",table, "' ");
  Result *pushidres = sqlRead(DAILY_TERMINATED, 0, 0 , 1, 1, getpushid);
  char *clause = cat(2, " ROWID > ",
		     (!pushidres->table)?itos(0):pushidres->table->row->val[3]);
  Result *res = sqlRead(table, 0, 0, 0, 0, clause);
  if (!res->table) {
    free(pushidres);
    free(res);
    return zerostate();
  }
  State *states = malloc(res->table->size*sizeof(State));
  tableres2states(res->table, states);
  State *restate = calloc(1, sizeof(State));
  if (states[0].type == start) {
    for (int i = 0; i < res->table->size -1; i++) {
      restate->val += difftime(mktime(states[i].date), mktime(states[i++].date));
    }
    restate->val/=60;
  }
  else if (states[0].type == event) {
    for (int i = 0; i < res->table->size; i++) {
      restate->val+=states[i].val;
    }
  }
  
  restate->date = malloc(sizeof(struct tm));
  memcpy(restate->date, states[0].date, sizeof(struct tm));
  restate->rowid = states[0].rowid;
  
  free(pushidres);
  free(states);
  free(res);
  return restate;
}

int startst(String ctable) {
    if(notexist(ctable)) {
        createTclk(ctable);
        highlight(cat(2, "new clock table created: ", ctable));
        addlinkable(ctable);
    }

    if(validstate(ctable, start))
        return insertts(ctable, 0, start);
    else error("unvalid state");
    return FAILED;
}

int stopst(String ctable) {
  if(notexist(ctable)) {
    error(cat(2, "can't push stop state, table doesn't exists with name: ", ctable));
    return none;
  }
  
  if (validstate(ctable, stop))
    return insertts(ctable, 0, stop);
  else error("unvalid state");
  return FAILED;
}

int eventst(String ctable, String dval) {
  if(notexist(ctable))
        createTclk(ctable);
  if(validstate(ctable, event))
        return insertts(ctable, dval, event);
  return FAILED;
}

State *zerostate() {
  State *state = malloc(sizeof(State));
  state->type = none;
  state->date = 0;
  state->val = 0;
  state->rowid = 0;
  return state;
}

//depricated
State *state(String ctable) {
  Result *res = sqlRead(ctable, 0, 0, 1, 1, 0);
  handleRes(res);

  if (res->type == errorres || !res->table)
    return zerostate();

  State *states = malloc(res->table->size*sizeof(State));
  tableres2states(res->table, states);
  free(res);
  return states;
}

State* last_state(String ctable) {
  return state(ctable);
}

/*int ssf_task_exist(String table) {
    String cols[] = { makeval(SSF) };
    sqlRead(SSF, )
    }*/

int ssf_start(String ctable) {
    if (notexist(SSF)) {
        Val cols[] =  { makeval(SSF_TASK, sdt_type) };
        assert(sqlCreate(SSF, cols, 1));
    }
    if (notexist(ctable)) { //todo fix
        Val vals[] = { makeval(ctable, sdt_type) };
        String cols[] = { SSF, SSF_TASK };
        assert(sqlInsert(cols, vals, 1));
    }
    return startst(ctable);
}

int ssf_stop(String ctable) {
    return stopst(ctable);
}

String* list_ssf_tasks() {
  Result *res = sqlReadFull(SSF);
  if (res->type == errorres || !res->table)
    return NULL;

  Row *row = res->table->row;
  String *tasks = malloc(sizeof(String)*(res->table->size+1));
  for (int r = 0; r < res->table->size; r++) {
    tasks[r] = strdup(row->val[2]);
    row = row->nxt; 
  }
  tasks[res->table->size] = NULL;
  free(res);
  return tasks;
}

String* list_current_ssf_tasks() {
    String *tasks = list_ssf_tasks();
    if (!tasks)
        return tasks;

    int cnt = 0;
    State *laststate;
    while (tasks[cnt]) {
      laststate = state(tasks[cnt]);
      if (laststate->type != start) {
	free(tasks[cnt]);
	tasks[cnt] = NULL;
      }
      cnt++;
      if (laststate->type != none) //not static struct
	free(laststate);
    }
    for (int i = 0; i < cnt; i++) {
      if (!tasks[i]) {
	for (int j = i+1; j < cnt && tasks[j]; j++) {
	  strcpy(tasks[i], tasks[j]);
	  free(tasks[j]);
	  tasks[j] = NULL;
	}
      }
    }
    return tasks;
}

extern Hist *hist;
int undost(String ctable, Hist *history) {
  if(!validstate(ctable, undo)) {
        error("can't undo, no prior record is present!");
        return FAILED;
  }
  State *st = state(ctable);
  hist = addhist(history, st);
  return deleteLastRow(ctable);
}

int redost(String ctable, Hist *hist) {
  if (hist == NULL || hist->size < 1) {
        error("can't redo, history is empty!");
        return FAILED;
  }
  Val vals[] = {
        makeval(tm2ts(hist->current->date), sdt_date),
        makeval(itos(hist->current->val), sdt_double),
        makeval(itos(hist->current->type), sdt_number),
  };
  pophist(hist);
  printf("redo val[0] %s\n", vals[0].strep);
  String names[] = {ctable, DATE, VAL, STATE};
  return sqlInsert(names, vals, 3);
}
