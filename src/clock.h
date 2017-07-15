#include <time.h>

#ifndef UTILS
#include "utils.h"
#endif
#define CLOCK

/*
  features
   - transactions handling
   - history, for undo, and redo.
   - accumulated accumulatable table

  ACCUMULATABLE TABLE
  ----------------------------
  |ROWID|D_DATE|N_VAL|N_STATE|
  ----------------------------
  !created extactly per givin name

  VAL_ACCUMULATED IF STATE IS even, push, otherwise
  it's DATE_ACCUMULATED.

*/

const int accumulatable;
const int naccumulatable;
String STATE;
String VAL;
String DATE; //default

enum statetype {
  event = 4, // VAL_ACCUMULATED
  start = 5, // DATE_ACCUMULATED
  stop = 6,  // ~
  push = 7,  // termination state
  none = -1,
  undo = -2, // functionality state.
};

typedef struct State {
  int type;
  struct tm *date;
  long double val;
  int rowid;
} State;

/* history is useful within short
   period and per table and gets nulled iff
   - new records for the same table
   - [OR] history creation for different table*/
typedef struct Hist {
  State *current;
  struct Hist *previous;
  int size;
} Hist;


/* Create ctable with givin name if not exists,
  validate and push start state */
int startst(String ctable);
/* validate and push stop state */
int stopst(String ctable);
/* for VALUED-ACCUMULATED TABLE */
int eventst(String ctable, String dval);
/* validate, push end state, calls accumulate*/
State *pushst(String ctable);
/* view last state */
State* last_state(String);
State* state(String ctable);
/* validate, remove last state iff {start, stop},
   expand and add new state to history*/
int ssf_start(String);
int ssf_stop(String);
String* list_ssf_tasks();
String* list_current_ssf_tasks();
int is_ssf(String);
int undost(String ctable, Hist *history);
/* redo and prune state histoy */
int redost(String ctable, Hist *history);
/* zerostate means table unfresh upon linking */
State *zerostate();
/* assumes table is fresh
   validate last state is push,
   accumulate last day data with start rowid */
State *accumulate (String table);
