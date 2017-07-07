#include "utils.h"
#include "assert.h"
#include <math.h>
#include "dirent.h"

char *dash = "-";
char *colon = ":";
char *space = " ";
String SQL_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";

String cat (int ignore, ...)
{
    va_list list;
    va_start(list, ignore);
    String arg, dest, olddest;

    olddest = va_arg(list, String);
    if(!olddest) olddest = "";
        while (ignore-- > 1)
        {
                arg = va_arg(list, String);
                if(arg == NULL || !strcmp(arg, "")) continue;
                dest = calloc(1, strlen(olddest)+strlen(arg)+1);
                strncat(dest, (const char*)olddest, strlen(olddest));
                strncat(dest, (const char*)arg, strlen(arg));
                strcat(dest, "\0");
                olddest = dest;
        }
        dest = calloc (1, strlen(olddest)+2);
        strncat(dest, olddest, strlen(olddest));
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
{dbg;
        int size, tradoff = 1;
        if(integer < 0) {
                integer *=-1;
                tradoff +=1;
        }
        size = (integer)?
                (int) log10((double)integer)
                :0
                + tradoff;
        char *buffer = malloc(size);
        sprintf(buffer, "%d", integer);
        return buffer;
}

String ftos(long double f)
{
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
        error(cat(2, "type not defined givin flag ",
                  itos(type)));
        exit(0);
    }
    return cat(2, pre, name);
}

int getDataType(String colName)
{dbg;
        if(!strcmp("ROWID", colName))
                return sdt_number;

        int type;
        switch(colName[0])
                {
        case 'T':
                type = sdt_type;
                break;
        case 'S':
                type = sdt_string;
                break;
        case 'B':
                type = sdt_blob;
                break;
        case 'D':
                type = sdt_date;
                break;
        case 'N':
                type = sdt_number;
                break;
        case 'L':
                type = sdt_double;
                break;
        default:
                type = sdt_number;
                }
        return type;
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
