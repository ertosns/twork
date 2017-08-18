#include <regex.h>
#include <string.h>
#include <assert.h>
#include <sqlite3.h>
#include <time.h>
#include "crud.h"
#ifndef DEBUG
#include "debug.h"
#endif

sqlite3 *database;
sqlite3_stmt *stm;
Table *table;
String cmdsuccess = "command succeeds";
String USR_DB;
String DEV_DB = "file:/tmp/twork/twork.db";

void initcrud() {
  String home = getenv("HOME");
  USR_DB = cat(3, "file:", home, "/twork/twork.db");
  initDB();
  table = NULL;
}

Table *copyTable();
Result *sqlprologue(int, String);

void initDB()
{
  int rc = sqlite3_open_v2((dbgmode)?DEV_DB:USR_DB, &database,
			   SQLITE_OPEN_READWRITE|
			   SQLITE_OPEN_CREATE|
			   SQLITE_OPEN_URI,
			   NULL);
  assert(rc == SQLITE_OK);
}

void finalizecrud() {
  if (database)
    sqlite3_close(database);
}

Err* handleError(int rc, String command)
{
  Err *err = malloc(sizeof(Err));
  if(rc != SQLITE_OK) {
    err->status = FAILED;
    err->command = strdup(command);
    err->err = strdup((char *)sqlite3_errmsg(database));
  }
  else
    err->status = SUCCESS;
  return err;
}

void handleRes(Result *res)
{
  if(res->type == errorres && res->err->status == FAILED) {
    error(cat(4, "commxand:\n", res->err->command,  "\nerror:\n", res->err->err));
    exit(1);
  }
  else if (res->type == errorres)
    des_err(res->err);
}


void viewTable()
{
  if (!table)
    return;

  Row *rw = table->row;
  for (int r = 0; r < table->size; r++)
    {
      for (int j = 0; j < table->ncol; j++) {
	char *val;
	if (table->coltype[j] == sdt_date) {
	  struct tm *date = malloc(sizeof(struct tm));
	  strptime(rw->val[j], SQL_DATE_FORMAT, date);
	  val = asctime(date);
	  if (val[strlen(val)-1] == '\n')
	    val[strlen(val)-1] = '\0';
	  free(date);
	}
	else
	  val = rw->val[j]?rw->val[j]:"";
	printf("%s%s", val, "|");
      }
      printf("\n");
      rw = rw->nxt;
    }
}

int readTable(void *pt, int argc, char **argv, char **colName)
{
  if(table == NULL) {
    table = calloc(1, sizeof(Table));
    table->ncol = argc;
    table->colname = malloc(argc * sizeof(String));
    table->coltype = malloc(argc * sizeof(int));
    for(int i = 0; i < argc; i++) {
      table->colname[i] = strdup(colName[i]);
      table->coltype[i] = getDataType(colName[i]);
    }
    table->row = malloc(sizeof(Row));
    table->current = table->row;
    table->row->val = malloc(argc * sizeof(String));
  }
  else {
    table->current->nxt = malloc(sizeof(Row));
    table->current->nxt->val = malloc(argc*sizeof(String));
    table->current = table->current->nxt;
  }
  
  for(int i = 0; i < argc; i++) {
    if(argv[i])
      table->current->val[i] = strdup(argv[i]);
    else
      table->current->val[i] = NULL;
  }
  table->size++;
  return 0;
}

int notexist(String tablename)
{
  if(!tablename)
    return SUCCESS;
  
  String sql = cat(3, "select * from sqlite_master where type = 'table' and name = '", tablename, "';");
  initDB();
  des_tbl(table);
  int rc = sqlite3_exec(database, sql, readTable, 0, 0);
  Result *res = sqlprologue(rc, sql);
  return table == NULL;
}

String getSqlDateType(int typeFlag)
{
  switch(typeFlag)
    {
    case sdt_date:      return "DATE";
    case sdt_type:      return "VARCHAR(15)";
    case sdt_string:    return "TEXT";
    case sdt_blob:      return "BLOB";
    case sdt_number:    return "INTEGER";
    case sdt_double:    return "REAL";
    default: {
      error("unkown SQL type flag");
      return NULL;
    }
    }
}

Val *makevalptr(String strep, int valtype)
{
  if(strep != NULL && strstr((const char*)strep,"_val") == strep+strlen(strep)-4) {
    error("can't accept table ending with _val");
    exit(1);
  }
  Val *val = malloc(sizeof(Val));
  if(strep)
    val->strep = strdup(strep);
  val->valtype = valtype;
  return val;
}

//debug
Val makeval(String strep, int valtype) {
  Val *ptr = makevalptr(strep, valtype);
  return *ptr;
}

void exec(int command, KeyVal *keyval, String tname, String clause)
{
  String *keys = malloc((keyval->size+1)*sizeof(String));
  Val *vals = malloc(keyval->size*sizeof(Val));
  keys[0] = tname;
  int count = 0;
  while(keyval->size)
    {
      if (command == createcmd) {
	vals[count].strep = keyval->key;
	vals[count].valtype = keyval->val[0].valtype;
      } else {
	vals[count] = *keyval->val;
	keys[count+1] = keyval->key;
      }
      count++;
      if(keyval->size == 1) break;
      keyval = keyval->parent;
    }
  switch(command)
    {
    case createcmd: {
      sqlCreate(tname, vals, count);
      break;
    }
    case insertcmd: {
      sqlInsert(keys, vals, count);
      break;
    }
    case updatecmd: {
      sqlUpdate(keys, vals, clause);
      break;
    }
    default: error("unvalid command!");
    }
}

int validpasscode(String passcode)
{return TRUE;}

void addblob(Blob *blobs, Val vals[], int index)
{}

Table *copyTable () {
    if(!table) return NULL;

    Table *tbl = malloc(sizeof(Table));
    tbl->size = table->size;
    tbl->ncol = table->ncol;
    tbl->coltype = malloc(table->ncol*sizeof(int));
    tbl->colname = malloc(table->ncol*sizeof(String));

    for(int i = 0; i < table->ncol; i++) {
      tbl->colname[i] = strdup(table->colname[i]);
      tbl->coltype[i] = table->coltype[i];
    }

    table->current = table->row;
    tbl->row = malloc(sizeof(Row));
    tbl->current = tbl->row;

    for (int index = 0; index < table->size; index++) {
      tbl->current->val =
	malloc(table->ncol*sizeof(String));
      for(int c = 0; c < table->ncol; c++) {
	if(!table->current->val[c]) {
	  tbl->current->val[c] = NULL;
	  continue;
	}
	
	tbl->current->val[c] = strdup(table->current->val[c]);
      }
      if(index != table->size-1) {
	table->current = table->current->nxt;
	tbl->current->nxt = malloc(sizeof(Row));
	tbl->current = tbl->current->nxt;
      } 
    }
    return tbl;
}

int sqlCreate(String name, Val vals[], int count)
{
  String dt;
  String sql = cat(3, "CREATE TABLE IF NOT EXISTS ", name," (D_DATE DATE DEFAULT (datetime('now', 'utc')) ");
  Result *res;
  int rc;
  if (count)
    sql = cat(2, sql, ", ");
  
  for (i = 0; i < count; i++)
    {
      dt = getSqlDateType(vals[i].valtype);
      sql = cat(4, sql, prependType(vals[i].strep, vals[i].valtype), " ", dt);
      if (i == count-1)
	break;
      else
	sql = cat(2, sql, ", ");
    }
  sql = cat(2, sql, ");");
  des_tbl(table);
  rc = sqlite3_exec(database, sql, 0, NULL, 0);
  res =  sqlprologue(rc, sql);
  des_res(res);
  return rc == SQLITE_OK;
}

Result *sqlReadFull(String name) {
    return sqlReadGrouped(name, 0, 0, 0, 0, 0, 0, 0, 0);
}

Result *sqlRead(String name, Val cols[], int size, int limit, int asc, String clause) {
    return sqlReadGrouped(name, cols, size, 0, 0, 0, limit, asc, clause);
}

Result *sqlReadGrouped(String name, Val cols[], int size, int groupby, int group[], String having, int limit, int asc, String clause)
{
  if(notexist(name)) {
    //error("table doesn't exists");
    return &(Result) {errorres, .err=&(Err){FAILED}};
  }
  String sql = "select ";
  if(!size)
    sql = cat(2, sql, "ROWID, * ");
  else
    for(i = 0; i < size; i++)
      {
	sql = cat(2,sql, prependType(cols[i].strep, cols[i].valtype));
	sql = (i != (size-1)) ?cat(2, sql, ", "):cat(2, sql, " ");
      }
  sql = cat(4, sql, "FROM ", name, " ");
  if(clause)
    sql = cat(4, sql, " WHERE ", clause, " ");
  if (groupby) {
    sql = cat(2, sql, "group by ");
    for (int i = 0; i < groupby; i++) {
      sql = cat(3, sql, cols[group[i]].strep, " ");
      if ( i == groupby-1)
	sql = cat(2, sql, ", ");
    }
  }
  if (groupby && having)
    sql = cat(3, sql, "having ", having, " ");
  if(asc)
    sql = cat(2, sql, " order by ROWID DESC ");
  if(limit)
    sql = cat(3, sql, " LIMIT ", itos(limit));
  sql = cat(2, sql, ";");
  des_tbl(table);
  int rc = sqlite3_exec(database, sql, readTable, NULL, 0);
  return  sqlprologue(rc, sql);
}

Result *readTables()
{
  String sql = cat(1, "SELECT name FROM my_db.sqlite_master WHERE type = 'table';");
  db(cat(2, "SQL: ", sql));
  des_tbl(table);
  int rc = sqlite3_exec(database, sql, readTable, NULL, 0);
  return sqlprologue(rc, sql);
}

int sqlInsert(String names[], Val vals[], int size)
{
  if(notexist(names[0])) {
    error("table doesn't exists");
    return FAILED;
  }
  int cblob = 0;
  Blob *blobs;
  String sql = cat(3, "INSERT INTO ", names[0], " ( ");
  for (i=0; i < size; i++)
    {
      sql = cat(2, sql, prependType(names[i+1], vals[i].valtype));
      if (i == size-1) sql = cat(2, sql, ") values ( ");
      else sql = cat(2, sql, ", ");
    }
  
  for (i=0; i < size; i++)
    {
      switch(vals[i].valtype)
        {
        case sdt_blob:
	  {
            sql = cat(2, sql, "?");
            cblob++;
	    addblob(blobs, vals, i);
	    break;
	  }
        case sdt_date:
	  {
            sql = cat(4, sql, "\'", (vals[i].strep)?
                      vals[i].strep:getDateTime(), "\'");
            break;
	  }
        case sdt_number:
	  {
            sql = cat(2, sql, vals[i].strep);
            break;
	  }
        default :
	  {
            sql = cat(4, sql, "\'", vals[i].strep, "\'");
	  }
        }
      sql = (i == size-1) ? cat(2, sql, ");")
	: cat(2, sql, ", ");
    }
  int rc = sqlite3_exec(database, sql, 0, 0, 0);
  for(i = 0; i < cblob; i++)
    {
      int res = sqlite3_bind_blob(stm, i,blobs[i].bytes,
				  blobs[i].size,
				  SQLITE_TRANSIENT);
      if ((res = sqlite3_step(stm)) != SQLITE_DONE)
	handleError(res, cat(2, "binding command ", sql));
    }
  Result *res = sqlprologue(rc, sql);
  des_res(res);
  return rc == SQLITE_OK;
}

//TODO fix INTEGER COLUMN TO DOUBLE
int sqlAlter(String names[], String reftable[], int type)
{
    if(notexist(names[0])) {
        error("table doesn't exists");
        return FAILED;
    }
    String sql = cat(8, "ALTER TABLE ", names[0], " ADD COLUMN ",
		     prependType(names[1], type)," ",
		     getSqlDateType(type),
		     (reftable)
		     ?cat(5," REFERENCES ", reftable[0],"(", reftable[1], ")"):" ", "  ;");
    
    db(cat(2, "SQL: ", sql));
    int rc = sqlite3_exec(database, sql, 0, 0, 0);
    Result *res = sqlprologue(rc, sql);
    des_res(res);
    return rc == SQLITE_OK;
}

int sqlUpdate(String names[], Val vals[], String clause)
{
    //update just one column, usually that is the case.
    if(notexist(names[0])) {
        error("table doesn't exists");
        return FAILED;
    }
    int valisblob = FALSE;
    String sql = cat(5, "UPDATE ", names[0]," SET ",
                     prependType(names[1], vals[0].valtype),
                     "=");
    switch(vals[i].valtype)
    {
    case sdt_blob:
        {
            sql = cat(2, sql, "?");
            valisblob = TRUE;
            break;
        }
    case sdt_date:
        {
            sql = cat(4, sql, "\'", getDateTime(), "\'");
            break;
        }
    case sdt_number:
        {
            sql = cat(2, sql, vals[i].strep);
            break;
        }
    default : sql = cat(4, sql, "\'", vals[i].strep, "\'");
    }
    if(clause != NULL)
        sql = cat(3, sql, " WHERE ", clause);
    sql = cat(2, sql, ";");
    int rc = sqlite3_exec(database, sql, 0, 0, 0);
    if(valisblob)
    {
        Blob *blob;
        addblob(blob, vals, 0);
        int res = sqlite3_bind_blob(stm, 1, blob->bytes,
                                    blob->size,
                                    SQLITE_TRANSIENT);
        if((res = sqlite3_step(stm)) != SQLITE_DONE)
            handleError(res, cat(0, "binding command ", sql));
    }
    Result *res =  sqlprologue(rc, sql);
    des_res(res);
    return rc == SQLITE_OK;
}

Result *sqlprologue(int rc, String sql) {
  Err *err = handleError(rc, sql);
  Result *res = malloc(sizeof(Result));
  if(err->status == FAILED) {
    res->type=errorres;
    res->err = err;
    handleRes(res);
  } else {
    res->type=tableres;
    res->table = copyTable();
  }
  des_ptr(&sql, 1); 
  return res;
}

int deleteTable(String name)
{
  //TODO delete from linkables
  if(notexist(name)) {
    error("table doesn't exists");
    return FAILED;
  }
  String sql = cat(3, "DROP TABLE IF EXISTS ", name, ";");
  int rc = sqlite3_exec(database, sql, 0, 0, 0);
  Result *res = sqlprologue(rc, sql);
  des_res(res);
  return rc = SQLITE_OK;
}

int deleteRecords(String name, String clause) {
  String sql = cat(5, "delete from ", name, " where ", clause, ";");
  initDB();
  int rc = sqlite3_exec(database, sql, 0, 0, 0);
  if (rc != SQLITE_OK) {
    error(cat (2, "can't del records\nErr: ", sqlite3_errmsg(database)));
    return FAILED;
  }
  return SUCCESS;
}

int deleteLastRow(String name)
{
  if(notexist(name)) {
    error("table doesn't exists");
    return FAILED;
  }
  if(nrow(name) == 1)
    deleteTable(name);
  String sql = cat(5, "DELETE FROM ", name, " WHERE ROWID = (SELECT MAX(ROWID) FROM ",name,");");
  int rc = sqlite3_exec(database, sql, 0, 0, 0);
  Result *res = sqlprologue(rc, sql);
  des_res(res);
  return rc == SQLITE_OK;
}

void handleregex(int status, regex_t *regt) {
  if(status) {
    char *buf = malloc(1024);
    regerror(status, regt, buf, 1024);
    error(cat(2, "extract columns regex failed! ", buf));
  }
}


String *acolumns(String tablename, int *size) {
    if (notexist(tablename)) {
        *size = 0;
        error("givin table doesn't exist");
        return NULL;
    }
    char *sql = cat(3, "PRAGMA table_info('", tablename,"');");
    des_tbl(table);
    int rc = sqlite3_exec(database, sql,readTable, NULL, 0);
    Err *err = handleError(rc, sql);
    assert(err->status);
    String *columns = malloc(table->size*sizeof(String));
    table->current = table->row;
    for (int i = 0; i < table->size; i++) {
      columns[i] = strdup(table->current->val[1]);
      table->current = table->current->nxt;
    }
    *size = table->size;
    return columns;
}

String *atrimedcolumns(String tablename, int *size) {
    String *cols = acolumns(tablename, size);
    for(int i = 0; i < *size; i++) {
        removeColFlag(cols[i]);
        //TODO remove suffixes&prefixes
        //trimsuffix(cols[i]);
    }
    return cols;
}

int colexist(String tablename, String colname) {
    int size;
    String *cols = atrimedcolumns(tablename, &size);
    for(int i = 0; i < size; i++)
        if(!strcmp(cols[i], colname))
            return SUCCESS;
    return FAILED;
}

int lastrowid(String table) {
    Result *res = sqlRead(table, 0, 0, 1, 1, 0);
    if (res->table)
        return (atoi(res->table->row->val[0]));
    return 0;
}

int nrow(String tablename) {
    Result *res = sqlReadFull(tablename);
    int size = (res->table)? res->table->size:0;
    des_res(res);
    return size;
}

Row *rowi(String tablename, int i) {
    Result *res = sqlReadFull(tablename);
    int size = (res->table)? res->table->size:0;
    if(i >= size && !res->table)
        return NULL;

    Row *row = res->table->row;
    for(int i = 0; i < size; i++) {
        row = row->nxt;
    }
    return row;
}

String colval(Result *res, String colname, int irow) {
    if(res->table) {
        error("empty table");
        return NULL;
    }
    if(irow >= res->table->size) {
        error("out of range irow");
        return NULL;
    }
    int c;
    for(c = 0; c < res->table->ncol; c++)
        if(!strcmp(colname, res->table->colname[c]))
            break;
    if(c == res->table->ncol) {
        error("column name unvalid");
        return NULL;
    }
    Row *row = res->table->row;
    for(int r; r < irow; r++)
        row = row->nxt;

    return row->val[c];
}

void des_val(Val *vals) {
  //TODO fix this
  if (vals);
    // free(vals);
}

void des_err(Err *err) {
  //TODO fix
  /*  if (!err)
    return;

  if(err->command)
    free(err->command);
  if(err->err)
    free(err->err);

    free(err);*/
}

void des_row(Row *row, int size, int ncol) {
  if (size == 0)
    return;

  if (row->nxt)
    des_row(row->nxt, --size, ncol);
  
  des_ptr(row->val, ncol);
  free(row);
}

void des_tbl(Table *tbl) {
  if (!tbl)
    return;

  des_row(tbl->row, tbl->size, tbl->ncol);
  if(tbl->coltype)
    free(tbl->coltype);
  if(tbl->colname)
    free(tbl->colname);

  free(tbl);
  table = NULL;
}
	  
void des_res(Result *res) {
  if (!res)
    return;
  
  des_err(res->err);
  des_tbl(res->table);
  free(res);
}

void des_ptr(String *ptr, int size) {
  if (!ptr)
    return;
  
  if (size > 0)
    for (int i = 0; i < size; i++) {
      if (ptr[i])
	free(ptr[i]);
    }
 
}
