#include "linker.h"

String LINKABLES = "LINKABLES";
String PUSH_ROW = "PUSH";
String NAME_COL = "NAME";
String CUR_TASK = NULL;

int initlinker() { 
  read_cur_task();
  return SUCCESS;
}

String read_open_task() {
  int size;
  String *linkables = listlinkables(&size);
  State *state;
  
  if(CUR_TASK) {
    free(CUR_TASK);
    CUR_TASK = NULL;
  }
  
  for (int i = 0; i < size; i++) {
    state = last_state(linkables[i]);
    if (state && state->type == start) {
      CUR_TASK = strdup(linkables[i]);
      des_strs(linkables, size);
      freestate(state);
      return CUR_TASK;
    }
    freestate(state);
  }
  des_strs(linkables, size);
  return NULL;
}

String read_cur_task() {
  State *state;
  String open = read_open_task();
  if (!open)
    return NULL;
  SSF *tree = read_tree(open, NULL);
  for (int i = 0; tree->children && tree->children[i]; i++) {
    state = last_state(tree->children[i]->name);
    if (state->type==start)
      open = strdup(tree->children[i]->name);
    freestate(state);
  }
  freessf(tree);
  return open;
}

String* listlinkables(int *size) {
  if (notexist(LINKABLES)) {
    *size = 0;
    return NULL;
  }
  
  Val cols[] = { makeval(NAME_COL, sdt_type) };
  Result *res = sqlRead(LINKABLES, cols, 1, 0, 0, 0);
  *size = (res->table)?res->table->size:0;
  if (!*size)
    goto des;
  String *linkables = malloc(*size*sizeof(String));
  Row *rw = res->table->row;
  for(int i = 0; i < *size ; i++) {
    linkables[i] = strdup(rw->val[0]);
    rw = rw->nxt;
  }

 des:
  des_res(res);
  free(cols[0].strep);
  return linkables;
}


int linkablexists(String name) {
  int size, exists = 0;
  String *linkables =  listlinkables(&size);
  for (int i = 0; i < size; i++)
    if (!strcmp(linkables[i], name))
      exists = 1;
  
  des_strs(linkables, size);
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
    free(lcols[0].strep);
    free(lcols[1].strep);
  }
  else if(linkablexists(tablename)) {
    error("givin table already exists in LINKABLES table");
    return FAILED;
  }
  
  String names[] = {LINKABLES, NAME_COL};
  Val val[] = { makeval(tablename, sdt_type) };
  int rc = sqlInsert(names, val, 1);
  free(val[0].strep);
  return rc;
}



int removelinkable(String table) {
  int success = 0;
  deleteTable(table);
  deleteRecords(LINKABLES, cat(4, prependType(NAME_COL, sdt_type), " = '", table, "' "));
  break_leaf(table);
  return 1;
}

