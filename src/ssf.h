#define SSF_H

#ifndef CRUD
#include "crud.h"
#endif

#ifndef CLOCK
#include "clock.h"
#endif



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

typedef struct SSF
{
  struct SSF* parent;
  struct SSF* children; /*null terminated*/
  String name;
  int state;
} SSF;

void add_branch(String node, String parent);
void break_branch(String node, String parent);
void break_leaf(String node);
/*
   start nodes starts it's parent in un-symmetric relation 
   return tree for prespective of given node, 
*/
SSF* read_tree(String node, SSF *tree);
void freessf(SSF *node);
