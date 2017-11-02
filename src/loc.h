/*
  code assumed to be organized under ${TWORK_DEVELOP} as submodules
  using git SCV with appropriate .gitignore which assumed to count only
  code,an manual authentication, push url configured. */
#define LOCHEADER

#ifndef LINKER
#include "linker.h"
#endif

#ifndef CLOCK
#include "clock.h"
#endif

int initloc();
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
