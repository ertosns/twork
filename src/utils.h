#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <dirent.h>
#ifndef DEBUG
#include "debug.h"
#include "../config.h"
#endif
#define UTILS
#define TRUE 1
#define FALSE 0
#define LEVEL VERBOSE
#define dbg                                                                     \
                if(LEVEL != NONE)                                               \
                        //highlight(cat(2,"--------->", __func__));
#define asize(array) sizeof(array)/sizeof(array[0])
int i, def_sdt_type, rule;
typedef char *String;
typedef long  Date[2];
enum {crt=1, ins=2, upt=3};
enum debug {NONE = 1, VERBOSE=2};
enum success {SUCCESS = 1, FAILED = 0};
enum response {errorres = 1, tableres = 2};
enum sqltype{createcmd=1,insertcmd=2,updatecmd=3,altercmd=4};
enum valtype {sqlvalue=1,sqltype=2};
enum datatype{sdt_type = 1,sdt_number = 2,sdt_double = 6,
              sdt_string = 3,sdt_blob = 4,sdt_date = 5};
enum floatype{FLOAT_NUM=1, DOUBLE_NUM=2, LDOUBLE_NUM=3};
typedef struct Blob
{
    String bytes;
        int size;
} Blob;
typedef struct Val
{
        String strep;
        int valtype;
} Val;
typedef struct KeyVal
{
        struct KeyVal *parent;
        String key;
        Val *val;
        int size;
} KeyVal;

/* every val read as string even blob 64base
  from storage prespective binary is optimum,
  fix if needed */

typedef struct Row
{
  struct Row *nxt;
  String *val;
} Row;

typedef struct
{
  int status;
  String command;
  String err;
} Err;

typedef struct Table
{
  Row *row; // start
  Row *current; // current reading|writing
  int size;
  int ncol;
  // ncol width
  int *coltype;
  String *colname;
  
} Table;

typedef struct Result
{
    int type;
    union
    {
        Err *err;
        Table *table;
    };
} Result;


String SQL_DATE_FORMAT;
int initutils();
String cat(int ignore, ...);
char* readFile(FILE *f);
String itos(int integer);
String ftos(long double f);
//append type flag for each column name, instead of running Pragma easier to implement
String prependType(String name, int type);
String removeColFlag(String name);
int getDataType(String name);
String tm2localstr(struct tm *info);
String tm2ts(struct tm *info);
String getDateTime();
int isdirectory(String);
int isfiler(String);
int isfilew(String);
int isfilerw(String);
int assertcmd(int *status);
