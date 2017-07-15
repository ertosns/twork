#include <math.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "clock.h"


char *dash = "-";
char *colon = ":";
char *space = " ";
String SQL_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";

void initutils() {

}

String cat (int ignore, ...)
{
  va_list list;
  String dest, arg;
  int Oignore=ignore, size=0;
  
  va_start(list, ignore);
  if(!ignore)
    return "";
  
  while (ignore-- > 0) {
    arg = va_arg(list, String);
    if (arg)
      size += strlen(arg);
  }
  va_start(list, ignore);
  
  dest = calloc(size+1, sizeof(char));
  while (Oignore-- > 0)
    {
      arg = va_arg(list, String);
      if(arg == NULL || !strcmp(arg, ""))
	continue;
      strncat(dest, (const char*)arg, strlen(arg));
    }
  
  va_end(list);
  return dest;
}


int floatdigitsize(long double point, enum floatype type)
{
  //TODO fix
  // handle zeros
  return 8;
}

String itos(int integer)
{
  int size, tradoff = 1;
  if(integer < 0) {
    integer *=-1;
    tradoff +=1;
  }
  size = (integer)? (int) log10((double)integer): 0 + tradoff;
  String buffer = malloc((size+1)* sizeof(char));
  sprintf(buffer, "%d", integer);
  return buffer;
}

String ftos(long double f)
{//TODO fix
  String buf = malloc(8*sizeof(char));
  sprintf(buf, "%LF", f);
  return buf;
}

String removeColFlag(String name)
{
  if(name[1] = '_')
    for(i = 2; i <= strlen(name); i++)
      name[i-2] = name[i];
  return name;
}

String prependType(String name, int type)
{
  String pre;
  switch(type)
    {
    case sdt_type:
      pre = "T_";
      break;
    case sdt_string:
      pre = "S_";
      break;
    case sdt_blob:
      pre = "B_";
      break;
    case sdt_date:
      pre = "D_";
      break;
    case sdt_number:
      pre = "N_";
      break;
    case sdt_double:
      pre = "L_";
      break;
    default:
      error(cat(2, "type not defined givin flag ", itos(type)));
      exit(0);
    }
  return cat(2, pre, name);
}

int getDataType(String colName)
{
  switch(colName[0])
    {
    case 'T':
      return sdt_type;
    case 'S':
      return sdt_string;
    case 'B':
      return sdt_blob;
    case 'D':
      return sdt_date;
    case 'N':
      return sdt_number;
    case 'L':
      return sdt_double;
    default: //include ROWID
      return sdt_number;
    }
  return sdt_number;
}

String tm2ts(struct tm *info) {
        char *datestr = malloc(20);
        strftime(datestr, 20, SQL_DATE_FORMAT, info);
        return datestr;
}

String getDateTime()
{
        time_t secs = time(NULL);
        struct tm *info = gmtime(&secs);
        return tm2ts(info);
}

int isdirectory(String path) {
        return opendir(path) != NULL;
}

int isfiler(String name) {
        return fopen(name, "r") != NULL;
}

int isfilew(String name) {
        return fopen(name, "w") != NULL;
}

int isfilerw(String name) {
        return fopen(name, "rw") != NULL;
}

int assertcmd(int *status) {
    //assume shell exists
    if(*status == -1)
        error("can't init child process");
    if(*status != 0)
        error(cat(2, "cmd failed termination status = ",
                  itos(*status)));
    *status = *status == 0;
    return *status;
}
