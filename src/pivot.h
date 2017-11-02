#define PIVOTHEADER

#ifndef CRUD
#include "crud.h"
#endif

#ifndef CLOCK
#include "clock.h"
#endif

#ifndef LINKER
#include "linker.h"
#endif
/*
  - only one pivot is allowed at a time for start, stop events
  - pivot assumed to have unique name
  - state {start, stop}

  PIVOT TABLE
  ||PIVOT||STATE||
*/


typedef struct Pivot {
  String name;
  struct tm *start_stamp;
  struct tm *stop_stamp;
} Pivot;

typedef struct cumulate {
  String name;
  int type;
  float tota;
} Cumulate;


void start_pivot(String);
void stop_pivot(String);
void del_pivot(String);
Pivot* get_pivot(String);
String* list_pivots(int *);
String* list_current_pivots(int*);
void freepiv(Pivot*);
Cumulate* cumulate_pivot(Pivot *piv, int *size);
String cumulate_pivot_str(Pivot *piv);
