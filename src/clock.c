#define _XOPEN_SOURCE
#define _GNU_SOURCE
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
#include "debug.h"

const int accumulatable = 1;
const int naccumulatable = 0;
String STATE = "STATE";
String VAL = "VAL";
String DATE = "DATE"; //default column
String SSF = "SSF";
String SSF_TASK = "SSF_TASK";
Hist *hist;

int initclock() {
  hist = calloc(1, sizeof(Hist));
  return SUCCESS;
}

void pophist(Hist *history) {
  if (history) {
    hist = history->previous;
    hist->size = history->size--;
    freestate(history->current);
    free(history);
  }
}

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

int undost(String ctable) {
  if(!validstate(ctable, undo)) {
    error("can't undo, no prior record is present!");
    return FAILED;
  }
  State *st = state(ctable);
  hist = addhist(hist, st);
  return deleteLastRow(ctable);
}

int redost(String ctable) {
  if (hist == NULL || hist->size < 1) {
    error("can't redo, history is empty!");
    return FAILED;
  }
  Val vals[] = {
    makeval(tm2localstr(hist->current->date), sdt_date),
    makeval(itos(hist->current->val), sdt_double),
    makeval(itos(hist->current->type), sdt_number),
  };
  pophist(hist);
  String names[] = {ctable, DATE, VAL, STATE};
  return sqlInsert(names, vals, 3);
}


String period2stamp(String ctable, long long sec) {
  State *state = last_state(ctable);
  struct tm *date = state->date; //utc
  time_t currentTime = time(NULL);//utc
  double diff = difftime(currentTime, timegm(date));
  diff = sec%(long)diff;
  time_t stamp = currentTime-diff;
  return tm2ts(gmtime(&stamp));
}

void createTclk(String ctable) {
  Val stateval[] = { makeval(VAL, sdt_double),
                     makeval(STATE, sdt_number)};
  sqlCreate(ctable, stateval, 2);
}

//TODO IMPLEMENT AUTO COL-NAME GRAPPER.
int insertts(String ctable, String dval, int state) {
  Val vals[] = { makeval((dval)?dval:"0", sdt_double),
                 makeval(itos(state), sdt_number) };
  String names[] = {ctable, VAL, STATE};
  return sqlInsert(names, vals, 2);
}

int lateinsertts(String ctable, String dval, int state, long long sec){
  Val vals[] = { makeval(period2stamp(ctable, sec), sdt_date),
                 makeval((dval)?dval:"0", sdt_double),
                 makeval(itos(state), sdt_number) };
  String names[] = {ctable, DATE, VAL, STATE};
  return sqlInsert(names, vals, 3);
}

String strstate(int state) {
  switch(state) {
  case event: return "event";
  case start: return "start";
  case undo: return "undo";
  case redo: return "redo";
  case stop: return "stop";
  case none: return "none";
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
    break;
  case start:
    if (st != stop && st != undo) {
      error(cat(2, "start state expecting stop but gets ", strstate(st)));
      return 0;
    }
    break;
  case stop:
    if (st != start && st != undo) {
      error(cat(2, "stop state expecting start but gets ", strstate(st)));
      return 0;
    }
    break;
  case event:
    if (st != event) {
      error(cat(2, "event expecting event but gets ", strstate(st)));
      return 0;
    }
    break;
  default:
      error("unkown state!");
    return FAILED;
  }
  freestate(laststate);
  return SUCCESS;
}

void freestate(State *state) {
  if (!state)
    return;
  if (state->date && state->type!=none)
    free(state->date);
  free(state);
}

void tableres2states(Table *table, State *states)
{
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

int latestate(String ctable, String dval, int state, long long sec) {
  if (notexist(ctable)) {
    highlight("can't late insert to not existing table!");
    return FAILED;
  }
  
  if(validstate(ctable, state))
    return lateinsertts(ctable, dval, state, sec);
  else error("unvalid state");
  
  return FAILED;
}

int startst(String ctable) {
  if (notexist(ctable)) {
    createTclk(ctable);
    addlinkable(ctable);
    highlight(cat(2, "new clock table created: ", ctable));
  }
  
  if(validstate(ctable, start))
    return insertts(ctable, 0, start);
  else error("unvalid state");
  return FAILED;
}

int stopst(String ctable) {
  if(notexist(ctable)) {
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

State *state(String ctable) {
  
  Result *res = sqlRead(ctable, 0, 0, 1, 1, 0);
  if (res->type == errorres || !res->table)
    return zerostate();
  handleRes(res);

  State *states = malloc(res->table->size*sizeof(State));
  tableres2states(res->table, states);
  free(res);
  return states;
}

State* last_state(String ctable) {
  return state(ctable);
}

int ssf_start(String ctable) {
    if (notexist(SSF)) {
        Val cols[] =  { makeval(SSF_TASK, sdt_type) };
        assert(sqlCreate(SSF, cols, 1));
    }
    if (notexist(ctable)) {
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
    if (!tasks[0])
      return NULL;
    
    int rm=0, len=0;
    State *laststate;
    while (tasks[len]) {
      laststate = state(tasks[len]);
      if (laststate->type != start) {
	free(tasks[len]);
	tasks[len] = NULL;
        rm++;
      }
      freestate(laststate);
      len++;
    }
    
    // O(len)
    for (int i = 0; i<len; i++) {
      if (tasks[i])
        continue;
      for (int j = i+1; j < len; j++) {
        if (tasks[j]) {
          tasks[i] = tasks[j];                      
          tasks[j] = NULL;
          i=j-1;
          break;
        }
      }
    }
    return tasks;
}


int is_ssf(String name) {
  String *ssfs = list_ssf_tasks();
  if(!ssfs)
    return FAILED;
  
  for (int i = 0; ssfs[i]; i++) {
    if (!strcmp(ssfs[i], name))
      return SUCCESS;
  }
  return FAILED;
}
