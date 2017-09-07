#include "crud.h"
#include <sqlite3.h>
#ifndef DEBUG
#include "debug.h"
#endif
#include "regex.h"
#include <string.h>
#include <assert.h>

static sqlite3 *database;
static sqlite3_stmt *stm;
static Table *table;
static char *err_msg;
String cmdsuccess = "command succeeds";
String USR_DB;
String DEV_DB = "file:/tmp/twork/twork.db";

void initcrud() {
    String home = getenv("HOME");
    assert(home);
    USR_DB = cat(3, "file:", home, "/twork/twork.db");
}

Err* handleError(int rc, String command, String err_msg)
{
        Err *err = malloc(sizeof(Err));
        if(rc != SQLITE_OK) {
                err->status = FAILED;
                err->command = malloc(strlen(command));
                strcpy(err->command, command);
                char *tmperr = (char *) sqlite3_errmsg(database);
                err->err = malloc(strlen(tmperr));
                strcpy(err->err, tmperr);
        }
        else {
                err->status = SUCCESS;
        }
        return err;
}

void handleRes(Result *res)
{
        if(res->type == errorres &&
           res->err->status == FAILED) {
                error(cat(4, "commxand:\n", res->err->command,
                                  "\nerror:\n", res->err->err));
                exit(1);
        }
        else if (res->type == tableres) {}
        else {
                error("unvalid res status");
                exit(1);
        }
        //free err{msg}, table
}

void viewTable()
{
        Row *rw = table->row;
        for (int r = 0; r < table->size; r++)
        {
                for (int j = 0; j < table->ncol; j++) {
                        char *val;
                        if (table->coltype[j] == sdt_date) {
                                struct tm *date =
                                        malloc(sizeof(struct tm));
                                strptime(rw->val[j], SQL_DATE_FORMAT,
                                                 date);
                            val = asctime(date);
                            if(val[strlen(val)-1] == '\n')
                                        val[strlen(val)-1] = '\0';
                                free(date);
                        }
                        else val = rw->val[j]?rw->val[j]:"";
                        printf("%s%s", val, "|");
                }
                printf("\n");
                rw = rw->nxt;
        }
}

int readTable(void *pt, int argc, char **argv, char **colName)
{
    int wasnull = 0;
    if(table == NULL) {
        wasnull = 1;
        table = calloc(1, sizeof(Table));
        table->ncol = argc;
        table->colname = malloc(argc*sizeof(String));
        table->coltype = malloc(argc*sizeof(int));

        for(int i = 0; i < argc; i++) {
            table->colname[i] = colName[i];
            table->coltype[i] = getDataType(colName[i]);
        }
        table->row = malloc(sizeof(Row));
        table->current = table->row;
        table->row->val = malloc(argc*sizeof(String));
    }

    if(!wasnull) {
        table->current->nxt = malloc(sizeof(Row));
        table->current->nxt->val =
            malloc(argc*sizeof(String));
        table->current = table->current->nxt;
    }

    for(int i = 0; i < argc; i++) {
        if(!argv[i]) continue;
        table->current->val[i] =
            malloc(strlen(argv[i]));
        strcpy(table->current->val[i],
               (const char*) argv[i]);
    }

    table->size++;
    return 0;
}

int notexist(String tablename)
{
    initDB();
    String sql = cat(3, "select * from sqlite_master where type = 'table' and name = '", tablename, "';");
    if(table) {
        free(table);
        table = NULL;
    }
    int rc = sqlite3_exec(database, sql, readTable,
                          NULL, &err_msg);
    int notext = table == NULL;
    free(table);
    table = NULL;
    return notext;
}

void initDB()
{
    int rc = sqlite3_open_v2((dbgmode)?DEV_DB:USR_DB, &database,
                             SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
                             SQLITE_OPEN_URI,
                             NULL);
    Err *err = handleError(rc, "open twork.db", NULL);
    assert(err != NULL);
    //TODO fix doesn't work
    //handleRes(&(Result) {1, err});
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
        if(strep != NULL &&
           strstr((const char*)strep,"_val") ==
           strep+strlen(strep)-4) {
                error("can't accept table ending with _val");
                exit(1);
        }
        Val *val = malloc(sizeof(Val));
        if(strep) {
                val->strep = malloc(strlen(strep));
                strcpy(val->strep, strep);
        }
        val->valtype = valtype;
        return val;
}

Val makeval(String strep, int valtype) {
        Val *val = makevalptr(strep, valtype);
        return *val;
}

KeyVal* makekeyval (String key, Val *val)
{
        KeyVal *kv = malloc(sizeof(KeyVal));
        kv->key = malloc(strlen(key));
        strcpy(kv->key, key);
        kv->val = val;
        kv->size = 1;
        return kv;
}

KeyVal* addkeyval(KeyVal *kv1, KeyVal *kv2)
{
        kv2->parent = kv1;
        kv2->size = kv1->size+1;
        return kv2;
}

void exec(int command, KeyVal *keyval, String tname, String clause)
{
    String *keys =
        malloc((keyval->size+1)*sizeof(String));
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
        tbl->colname[i] =
            malloc(strlen(table->colname[i]));
        strcpy(tbl->colname[i], table->colname[i]);
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

            tbl->current->val[c] =
                malloc(strlen(table->current->val[c]));
            strcpy(tbl->current->val[c],
                   table->current->val[c]);
        }

        if(index != table->size-1) {
            table->current = table->current->nxt;
            tbl->current->nxt = malloc(sizeof(Row));
            tbl->current = tbl->current->nxt;
        }

    }
    return tbl;
}

Result *sqlprologue(int rc, String sql) {
    sqlite3_close(database);
    Err *err = handleError(rc, sql, err_msg);
    sqlite3_free(err_msg);
    Result *res = malloc(sizeof(Result));
    if(err->status == FAILED) {
        res->type=errorres;
        res->err = err;
        handleRes(res);
    } else {
        res->type=tableres;
        res->table = copyTable();
    }
    return res;
}

int sqlCreate(String name, Val vals[], int count)
{
    initDB();
    String sql = cat(3, "CREATE TABLE IF NOT EXISTS ", name," (D_DATE DATE DEFAULT (datetime('now', 'utc')) ");
    if (count)
        sql = cat(2, sql, ", ");
    String dt;
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
    if (table) {
        free(table);
        table = NULL;
    }
    int rc = sqlite3_exec(database, sql, 0, NULL, &err_msg);
    Result *res =  sqlprologue(rc, sql);
    handleRes(res);
    free(res);
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
<<<<<<< HEAD
    if(notexist(name)) {
        error("table doesn't exists");
        return &(Result) {errorres, .err=&(Err){FAILED}};
    }
    initDB();
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
    if(table) {
        free(table);
        table = NULL;
    }
    int rc = sqlite3_exec(database, sql, readTable, NULL,&err_msg);
    return  sqlprologue(rc, sql);
=======
  Result *nullRes = &(Result) {errorres, .err=&(Err){FAILED}};
  if(notexist(name)) {
    return nullRes;
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
>>>>>>> 9f01479... light alarm
}

Result *readTables()
{
    initDB();
    String sql = "SELECT name FROM my_db.sqlite_master WHERE type = 'table';";
    db(cat(2, "SQL: ", sql));
    if(table) {
        free(table);
        table = NULL;
    }
    if(table) {
        free(table);
        table = NULL;
    }
    int rc = sqlite3_exec(database, sql, readTable, NULL, &err_msg);
    return sqlprologue(rc, sql);
}

int sqlInsert(String names[], Val vals[], int size)
{
    if(notexist(names[0])) {
        error("table doesn't exists");
        return FAILED;
    }

    int cblob = 0;
    initDB();
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
    int rc = sqlite3_exec(database, sql, 0, 0, &err_msg);
    for(i = 0; i < cblob; i++)
    {
        int res = sqlite3_bind_blob(stm, i,blobs[i].bytes,
                                    blobs[i].size,
                                    SQLITE_TRANSIENT);
        if ((res = sqlite3_step(stm)) != SQLITE_DONE)
            handleError(res, cat(2, "binding command ",
                                 sql), err_msg);
    }

    Result *res = sqlprologue(rc, sql);
    handleRes(res);
    free(res);
    return rc == SQLITE_OK;
}

//TODO fix INTEGER COLUMN TO DOUBLE
int sqlAlter(String names[], String reftable[], int type)
{
    if(notexist(names[0])) {
        error("table doesn't exists");
        return FAILED;
    }
    initDB();
    String sql =
        cat(8, "ALTER TABLE ", names[0], " ADD COLUMN ",
            prependType(names[1], type)," ",
            getSqlDateType(type),
            (reftable)
            ?cat(5," REFERENCES ", reftable[0],"(",
                 reftable[1], ")"):" ", "  ;");

    db(cat(2, "SQL: ", sql));
    if(table) {
        free(table);
        table = NULL;
    }
    int rc = sqlite3_exec(database, sql, 0, 0, &err_msg);
    Result *res = sqlprologue(rc, sql);
    handleRes(res);
    free(res);
    return rc == SQLITE_OK;
}

int sqlUpdate(String names[], Val vals[], String clause)
{
    //update just one column, usually that is the case.
    if(notexist(names[0])) {
        error("table doesn't exists");
        return FAILED;
    }
    initDB();
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
    int rc = sqlite3_exec(database, sql, 0, 0, &err_msg);
    if(valisblob)
    {
        Blob *blob;
        addblob(blob, vals, 0);
        int res = sqlite3_bind_blob(stm, 1, blob->bytes,
                                    blob->size,
                                    SQLITE_TRANSIENT);
        if((res = sqlite3_step(stm)) != SQLITE_DONE)
            handleError(res, cat(0, "binding command ",
                                 sql), err_msg);
    }
    Result *res =  sqlprologue(rc, sql);
    handleRes(res);
    free(res);
    return rc == SQLITE_OK;
}

int deleteTable(String name)
{
    //TODO delete from linkables
    if(notexist(name)) {
        error("table doesn't exists");
        return FAILED;
    }
    initDB();
    String sql = cat(3, "DROP TABLE IF EXISTS ", name, ";");
    int rc = sqlite3_exec(database, sql, 0, 0, &err_msg);
    Result *res = sqlprologue(rc, sql);
    handleRes(res);
    free(res);
    return rc = SQLITE_OK;
}

int deleteRecords(String name, String clause) {
    initDB();
    String sql = cat(5, "delete from ", name, " where ", clause, ";");
    int rc = sqlite3_exec(database, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        error(cat (2, "can't del records\nErr: ", err_msg));
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
    initDB();
    String sql = cat(5, "DELETE FROM ", name,
                     " WHERE ROWID = (SELECT MAX(ROWID) FROM "
                     , name, ");");
    int rc = sqlite3_exec(database, sql, 0, 0, &err_msg);
    Result *res = sqlprologue(rc, sql);
    handleRes(res);
    free(res);
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
    initDB();
    if (table) {
        free(table);
        table = NULL;
    }
    int rc = sqlite3_exec(database, sql,readTable,
                          NULL, &err_msg);
    Err *err = handleError(rc, sql, err_msg);
    sqlite3_free(err_msg);
    assert(err->status);
    String *columns = malloc(table->size*sizeof(String));
    table->current = table->row;
    for (int i = 0; i < table->size; i++) {
        columns[i] = malloc(strlen(table->current->val[1]));
        strcpy(columns[i], table->current->val[1]);
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
    free(res);
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
