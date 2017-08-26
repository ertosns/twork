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

void initlinker();
String* listlinkables(int *size);
/* list of linkable tables */
int addlinkable(String tablename);
/* remove table with givin name from LINKABLES, DAILYTERM */
int removelinkable(String tablename);
/* get linkable type {event, start/stop}*/
int linkabletype(String name);
/* check if table fresh (has new records to link)
   fetch linkable values, from linkable tables,
   insert to DAY-TERMINATED DT table */
int link_accumulatables();
String read_cur_task();
