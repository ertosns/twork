#ifndef CRUD
#include <crud.h>
#endif
#ifndef UTILS
#include <utils.h>
#endif
#ifndef CLOCK
#include <clock.h>
#endif


/*
  - ssf is a forest not a tree.
  - each node has a unique name.
  - each node has one parent, and arbitrary children.
  - each node_name is name of valid table name under linkables.
  - if node is active, then it's parent is active.
  - ssf has to be start/stop kind of table.
  SSF_TABLE 
  ||timestamp||parent_name||node_name||state||

*/

struct SSF {
  String name;
  int state;
  SSF *parent; /*null iff tree root*/
  SSF *childs; /*null terminated*/
} SSF;
