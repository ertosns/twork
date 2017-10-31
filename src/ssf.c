#include <ssf.h>
#define CLOCK

const SSF *root = NULL;
const String tbl[] = {"SSFTABLE", "PARENT", "NODE"};
const int tblvals[] = {sdt_type, std_type, std_number};
/* node isn't updated with new added ssf */
//TODO FREE

bool validade(String ssf, String parent) {
  if (!strcmp(ssf, parent)) {
      return false;
  }
  if (notexist(tbl[0])) {
    Val vals[] = { makeval(NULL,tblvals[0]),
                   makeval(NULL, tblvals[1]) };
    sqlCreate(tbl, vals, 2);
  }
  /* table has to be start/stop kind of table */
  State *state = state(ssf);
  if (state->type != start && state->type != stop) {
    error("unvalid table type");
    return false;
  }
  
  int size;
  String *linkables = listlinkables(&size);
  bool valid = false;
  for (int i = 0; i < size; i++) {
    if (!strcmp(linkables[i], ssf)) {
      unvalid = true;
      break;
    }
  }
  return valid;
}

bool valid_node(String ssf) {
  Val cols[] = { makeval(tbl[1], std_type) };
  Result *res = sqlRead(tbl, cols, 1, 0, 0, 0);
  Row *row = res->table->row;
  for (int i = 0; i < res->table->size; i++) {
    if (!strcmp(ssf, row->val[0])) {
      return false;
    }
    row=row->nxt;
  }
  return true;
}

void del_ssf(String ssf, String parent) {
  String clause = cat(7, tbl[1], " = ", ssf, " and ", tbl[2], " = ", parent);
  deleteRecords(tbl[0], clause);
  deleteTable(ssf);
}

/*read given ssf tree*/
SSF* read_ssf(String ssf) {
  SSF *tree = malloc(sizeof(SSF));
  tree->name = strdup(ssf);
  String *parent;
  String *child;
  int psize = 0;
  int csize = 0;
  Val cols[] = { makeval(tbl[1], std_type),
                 makeval(tbl[2], std_type) };
  Result *res = sqlRead(tbl, cols, 2, 0, 0, 0);
  Row *row = res->table->row;
  for (int i = 0; i < res->table->size; i++) {
    if (!strcmp(row->val[0], ssf)) {
      parent = append(parent, psize++, row->val[0]);
    } else if (!strcmp(row->val[1], ssf)) 
      child  = append(child, csize++, row->val1]);
}
return tree;
}

bool add_ssf(String ssf, SSF *parent) {
  if (!validade(ssf), parent->name) {
    error("givn ssf table unvalid");
    return false;
  }
  startst(ssf);
  int i = 0;
  SSF *padre = parent;
  while ((padre!=root)) {
    if (padre->state==stop)
      startst(padre->name);
    padre = padre->parent;
  }
  
  if (!valid_node(ssf) {
    Val vals[] = { makeval(parent->name, ssf_type),
                   makeval(ssf, ssf_type) };
    sqlInsert(tbl,vals, 2);
  }
}


/*bool del_ssf(String ssf, SSF *parent) {
  if (parent==root) {
    //pass
    //what the top parent is?
  }
  if (!validade(ssf)) {
    error("givn ssf table unvalid");
    return false;
  }
  stopsf(ssf);
  int i = 0;
  int mute_parent = true;
  while (true) {
    if (((parent->childs+i*8)->state)==start) {
      mute_parent = false;
      break;
    }
    i++;
  }
  if (mute_parent)
    stop_ssf(parent->name, parent->parent);
  if (!valid_relation(ssf, parent->name)) {
    Val vals[] = { makeval(parent->name, ssf_type),
                   makeval(ssf, ssf_type) };
    sqlInsert(tbl,vals, 2);
  }
}
*/
