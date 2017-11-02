#define ATRACK
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef UTILS
#include "utils.h"
#endif

typedef struct session {
    String date;
    String task;
    String window;
    int keying;
    int mousing;
} session;

session *cursession;

// database session table
const String SESSIONS; //table name
const String START_DATE;
//! note that end date is the default date column
const String LINKABLE_TASK;
const String WINDOW;
const String KEYING_FREQ;
const String MOUSING_FREQ;

// database window table
const String WINDOW;
const String WINDOW_NAME;
/* initialize variables */
void initatrack();
/* infinite background tracking looping process,
   block for user interaction */
int track();
