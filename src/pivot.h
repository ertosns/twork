#include <crud.h>
#include <clock.h>
#include <time.h>
#include <stdlib.h>

const String tbl[];

typedef stuct Pivot {
  Stirng name;
  String des;
  tm *start_stamp;
  tm *stop_stamp;
} Pivot;

typedef struct cumulate {
  String name;
  int type;
  float tota;
} Cumulate;


void start_pivot(String, String);
void stop_pivot(String);
void del_pivot(Pivot *);
Pivot* get_pivot(String);
String* list_pivots(int *);
Cumulate* cumulate_pivot(Pivot *piv, int *size);
String cumulate_pivot_str(Pivot *piv);
