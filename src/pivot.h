#include <crud.h>
#include <clock.h>
#include <time.h>
#include <stdlib.h>

/*
  - only one pivot is allowed at a time for start, stop events
  - pivot assumed to have unique name
  - state {start, stop}

  PIVOT TABLE
  ||PIVOT||STATE||
*/

const String tbl[];

typedef stuct Pivot {
  Stirng name;
  tm *start_stamp;
  tm *stop_stamp;
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
String* list_current_pivots(int*0;
void freepiv(Pivot*);
Cumulate* cumulate_pivot(Pivot *piv, int *size);
String cumulate_pivot_str(Pivot *piv);
