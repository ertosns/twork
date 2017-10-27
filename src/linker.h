#ifndef CRUD
#include "crud.h"
#endif
#ifndef CLOCK
#include "clock.h"
#endif
#define LINKER


String LINKABLES;
String DAILY_TERMINATED;
String NAME_COL;
String TYPE_COL;
String CUR_TASK;

int initlinker();
String* listlinkables(int *size);
/* list of linkable tables */
int addlinkable(String tablename);
/* remove table with givin name from LINKABLES, DAILYTERM */
int removelinkable(String tablename);
/* get linkable type {event, start/stop}*/
int linkabletype(String name);

String read_cur_task();
