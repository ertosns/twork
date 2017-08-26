#include <string.h>
#include "linker.h"
#ifndef LOCHEADER
#include "loc.h"
#endif

String LINKABLES = "LINKABLES";
String DAILY_TERMINATED = "DAILYTERM";
String PUSH_ROW = "PUSH";
String NAME_COL = "NAME";
String CUR_TASK = NULL;
//TODO change word limit {colName}
//TODO loc fix
//compare & check strange bug
void initlinker() { 
  read_cur_task(); 
}

String* listlinkables(int *size) {
  if (notexist(LINKABLES)) {
    *size = 0;
    return NULL;
  }
  
  String *linkable = NULL;
  Val cols[] = { makeval(NAME_COL, sdt_type) };
  Result *res = sqlRead(LINKABLES, cols, 1, 0, 0, 0);
  *size = (res->table)?res->table->size:0;
  if (!*size)
    goto des;
  
  linkable = malloc(*size * sizeof(String));
  Row *rw = res->table->row;
  for(int i = 0; i < *size ; i++) {
    linkable[i] = strdup(rw->val[0]);
    rw = rw->nxt;
  }

 des:
  des_res(res);
  des_val(&cols[0]);
  return linkable;
}


int linkablexists(String name) {
  int size, exists = 0;
  String *linkables =  listlinkables(&size);
  for (int i = 0; i < size; i++)
    if (!strcmp(linkables[i], name))
      exists = 1;
  
  des_ptr(linkables, size);
  return exists;
}

int linkabletype(String name) {
  int type = 0;
  Result *res = NULL;
  if (!linkablexists(name))
    goto des;

  res = sqlRead(name, NULL, 0, 1, 0, 0);
  handleRes(res);
  if(!res->table)
    goto des;

  type = atoi(res->table->row->val[3]);
  if (type == start || type == stop)
        type = start;
  else type = event;

 des:
  des_res(res);
  return type;
}

int addlinkable(String tablename) {
  if(notexist(tablename)) {
    printf("givin linkable %s doesn't exist!\n", tablename);
    return FAILED;
  }
  
  if(notexist(LINKABLES)) {
    Val lcols[] = { makeval( NAME_COL, sdt_type) };
    sqlCreate(LINKABLES, lcols, 1);
    Val dtcols[] = { makeval(NAME_COL, sdt_type),
		     makeval(PUSH_ROW, sdt_number) } ;
    sqlCreate(DAILY_TERMINATED, dtcols, 2);
    des_val(&lcols[0]);
    des_val(&lcols[1]);
  }
  else if(linkablexists(tablename)) {
    error("givin table already exists in LINKABLES table");
    return FAILED;
  }
  
  String names[] = {LINKABLES, NAME_COL};
  Val val[] = { makeval(tablename, sdt_type) };
  int rc = sqlInsert(names, val, 1);
  des_val(&val[0]);
  return rc;
}

int isfresh(String name) {
  int fresh = 0;
  Result *linkableres = NULL;
  Result *dtres = NULL;
  String targetTable;
  if (notexist(name))
    goto des;
  
  linkableres = sqlReadFull(name);
  if(!linkableres->table)
    goto des;
  
  targetTable = cat(8, prependType(NAME_COL, sdt_type), " = '", name,  "' AND ", prependType(PUSH_ROW, sdt_number), " >= ", itos(lastrowid(name)), " ");
  dtres = sqlRead(DAILY_TERMINATED, 0, 0, 1, 1, targetTable);
  fresh = dtres->table == NULL;
  
 des:
  des_res(linkableres);
  des_res(dtres);
  des_ptr(&targetTable, 1);
  return fresh;
}

int removelinkable(String table) {
  int success = 0;
  deleteTable(table);
  deleteRecords(LINKABLES, cat(4, prependType(NAME_COL, sdt_type), " = '", table, "' "));
  deleteRecords(DAILY_TERMINATED, cat(4, prependType(NAME_COL, sdt_type), " = '", table, "' "));
  return 1;
}


int link_accumulatables() {
  int size, suc;
  String *linkables = listlinkables(&size);
  String cols[] = { DAILY_TERMINATED, NAME_COL, PUSH_ROW };
  for (int i = 0; i < size; i++) {
    if (isfresh(linkables[i])) {
	Val vals[] = { makeval(linkables[i], sdt_type),
		       makeval(itos(lastrowid(linkables[i])), sdt_number) };
	suc = sqlInsert(cols, vals, 2);
	des_val(&vals[0]);
    }
  }
  if (!dbgmode)
    loc(NULL);

  des_ptr(linkables, size);
  return suc;
}


String read_cur_task() {
  int size;
  String *linkables = listlinkables(&size);
  State *state;
  
  if(CUR_TASK) {
    free(CUR_TASK);
    CUR_TASK = NULL;
  }
  
  for (int i = 0; i < size; i++) {
    state = last_state(linkables[i]);
    if (state && state->type == start && !is_ssf(linkables[i])) {
      CUR_TASK = strdup(linkables[i]);
      free(linkables[i]);
      return CUR_TASK;
    }
    free(linkables[i]);
  }
  return NULL;
}
