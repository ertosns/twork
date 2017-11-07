#ifndef UTILS
#include "utils.h"
#endif


String ROWID;
String start_str;
String stop_str;
String event_str;

int initcrud();
/* check givin rc and return error with according
  error message if found */
Err* handleError(int rc, String command);
/* in case of error halt it's programmer error */
void handleRes(Result *res);
/* map flag sdt_type to sqlite definition */
void finalizecrud();
String getSqlDateType(int typeFlag);
/* add table name */
void viewTable();
void exec(int command, KeyVal *keyval, String name, String clause);
Val* makevalptr(String val, int valtype);
Val makeval(String val, int valtype);
/*//form keyval with NULL parent.
KeyVal *makekeyval (String key, Val *val);
//attach kv1 ref as parent to kv2, and return kv2.
KeyVal *addkeyval(KeyVal* kv1, KeyVal* kv2);
*/
void initDB();
int sqlCreate(String name, Val vals[], int count);
Result* sqlRead(String table, Val cols[], int count, int limit, int asc, String clause);
Result* sqlReadFull(String table);
Result* sqlReadGrouped(String table,  Val cols[], int count, int groupby, int group[], String having, int limit, int asc, String clause);
int sqlInsert(String names[], Val vals[], int size);
int sqlAlter(String names[], String reftable[], int type);
int sqlUpdate(String names[], Val vals[], String clause);
String readlastrecords(String, String, String);
Result* getTables();

int deleteTable(String name);
int deleteRecords(String name, String clause);
/*table assumed to have at least one record*/
int deleteLastRow(String name);
String droplastrow(String tname, String passcode);
String droptable(String tname, String passcode);

int notexist(String tablename);
String* acolumns(String, int *);
String* atrimedcolumns(String, int *);
int colexist(String tname, String col);
/* irow is zerobased, unvalid returns NULL */
Row* rowi(String, int);
/* r is zerobased */
String colval(Result*, String colname, int r);
int lastrowid(String);
int nrow(String);
void des_res(Result *res);
void des_tbl(Table *tbl);
void des_row(Row *row, int size, int ncol);
void des_err(Err *err);
void des_val(Val *);
void des_strs(String *, int size);
