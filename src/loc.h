#include <stdlib.h>
#ifndef LINKER
#include "linker.h"
#endif
#define LOC_HDR

void initloc();
/*get last commited hash*/
String getcomithash();
/*git diff between last locally stored commits
  return:
  index[0] addition
  index[1] deletion
 */
int *comitdiff();


/*add, commit with givin msg, push, store yield loc*/
int loc(String);
