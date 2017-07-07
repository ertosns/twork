#include "linker.h"
#ifndef LOCHEADER
#include "loc.h"
#endif

String LINKABLES = "LINKABLES";
String DAILY_TERMINATED = "DAILYTERM";
String PUSH_COL = "PUSH";
String NAME_COL = "NAME";

//TODO change word limit {colName}
//TODO loc fix
//compare & check strange bug
String* listlinkables(int *size) {
  if(notexist(LINKABLES)) {
        *size = 0;
        return NULL;
  }

  Val cols[] = { makeval(NAME_COL, sdt_type) };
  Result *res = sqlRead(LINKABLES, cols, 1, 0, 0, 0);
  *size = (res->table)?res->table->size:0;
  if (!*size) {
      free(res);
      return NULL;
  }

  String *linkable = malloc(*size * sizeof(String));
  Row *rw = res->table->row;
  for(int i = 0; i < *size ; i++) {
      linkable[i] = malloc(strlen(rw->val[0]));
      strcpy(linkable[i], rw->val[0]);
      rw = rw->nxt;
  }
  free(res);
  return linkable;
}


int linkablexists(String name) {
    int size, exists = 0;
    String *linkables =  listlinkables(&size);
    for (int i = 0; i < size; i++)
        if (!strcmp(linkables[i], name))
            exists = 1;

    free(linkables);
    return exists;
}

int linkabletype(String name) {
  if(!linkablexists(name))
        return 0 ;

  Result *res = sqlRead(name, NULL, 0, 1, 0, 0);
  handleRes(res);
  if(!res->table) {
      free(res);
      return 0;
  }

  int type = atoi(res->table->row->val[3]);
  if(type == start || type == stop)
        type = start;
  else type = event;

  free(res);
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
                         makeval(PUSH_COL, sdt_number) } ;
        sqlCreate(DAILY_TERMINATED, dtcols, 2);
    }
    else if(linkablexists(tablename)) {
        error("givin table already exists in LINKABLES table");
        return FAILED;
    }

    String names[] = {LINKABLES, NAME_COL};
    Val val[] = { makeval(tablename, sdt_type) };
    return sqlInsert(names, val, 1);
}

int isfresh(String name) {
    if (notexist(name))
        return FAILED;
    Result *linkableres = sqlReadFull(name);
    if(!linkableres->table) {
        free(linkableres);
        return FAILED;
    }
    String targetTable = cat(8, prependType(NAME_COL, sdt_type), " = '", name,  "' AND ", prependType(PUSH_COL, sdt_number), " >= ", itos(lastrowid(name)), " ");
    Result *dtres = sqlRead(DAILY_TERMINATED, 0, 0, 1, 1, targetTable);
    return dtres->table == NULL;
}

int removelinkable(String table) {
    int success = 0;
    success += deleteTable(table);
    success += deleteRecords(LINKABLES, cat(4, NAME_COL, " = '", table, "' "));
    success += deleteRecords(DAILY_TERMINATED, cat(4, NAME_COL, " = '", table, "' "));
    return success == 3;
}


int link_accumulatables() {
    int size, suc;
    String *linkables = listlinkables(&size);
    String cols[] = { DAILY_TERMINATED, NAME_COL, PUSH_COL };
    for (int i = 0; i < size && isfresh(linkables[i]); i++) {
        Val vals[] = { makeval(linkables[i], sdt_type),
                       makeval(itos(lastrowid(linkables[i])), sdt_number) };
        suc = sqlInsert(cols, vals, 2);
    }
    if (getenv(TWORK_DEVELOP) && !dbgmode)
        loc(NULL);
    return suc;
}
