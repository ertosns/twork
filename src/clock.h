#include <time.h>
#ifndef UTILS
#include "utils.h"
#endif
#define CLOCK

/*
  features
   - events transactions
   - history for undo/redo.

   clocked TABLE
  ----------------------------
  |ROWID|D_DATE|N_VAL|N_STATE|
  ----------------------------
*/

String STATE;
String VAL;
String DATE; //default

enum statetype
  { event = 4,/*has numerical value*/ start = 5, stop = 6, none = 0, undo = -1, redo = -2  };

typedef struct State {
  int type;
  struct tm *date;
  long double val;
  int rowid;
} State;

/* history is useful within short
   period and per table and gets nulled iff
   - new records for the same table
   || history creation for different table*/
typedef struct Hist {
  State *current;
  struct Hist *previous;
  int size;
} Hist;


int initclock();
/*insert state with date happend at most before min minutes.*/
int latestate(String ctable, String dval, int state, long long sec);
/* Create ctable with givin name if not exists,
  validate and push start state */
int startst(String ctable);
/* validate and push stop state */
int stopst(String ctable);
/* for VALUED-CUMULATED TABLE */
int eventst(String ctable, String dval);
State* state(String ctable);
/* view last state */
State* last_state(String);
/* validate, remove last state iff {start, stop},
   expand and add new state to history*/
int ssf_start(String);
int ssf_stop(String);
String* list_ssf_tasks();
String* list_current_ssf_tasks();
int is_ssf(String);
int undost(String ctable);
/* redo and prune state histoy */
int redost(String ctable);
void freestate(State *state);
/* zerostate means table unfresh upon linking */
State *zerostate();
int validstate(String ctable, int st);
/* cumulate clocked table values time for start/stop, or occurances for evented table */
float cumulate (String ctable, tm *start_stamp, tm *stop_stamp, int *type);
