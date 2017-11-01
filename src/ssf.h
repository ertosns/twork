#ifndef CRUD
#include <crud.h>
#endif
#ifndef UTILS
#include <utils.h>
#endif
#ifndef CLOCK
#include <clock.h>
#endif
#define SSF

/*
  - SSF is a forest not a tree, or start/stop kind of table
  - each node has a unique name of valid clocked table name under linkables.
  - each node has one parent, and arbitrary children.
  - node can't equal it's parent.
  - if node is active, then it's parent is active.
  - results: 
    - time spend in node >+ total time spend on it's nodes.
    - only one leaf is active at a time.
  

  SSF_TABLE 
  ||timestamp||node_name||parent_name||state||

*/

const int ROOT;
const int LEAF;

struct SSF {
  String name;
  int state;
  SSF *parent;
  SSF *children; /*null terminated*/
} SSF;

void add_branch(String node, String parent);
void break_branch(String node, String parent);
void break_leaf(String node);
/* 
   start nodes starts it's parent in un-symmetric relation 
   return tree for prespective of given node, 
*/
SSF* read_tree(String node, SFF *tree);
void freessf(SSF *node);
