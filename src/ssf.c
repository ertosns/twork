#include "ssf.h"

String columns[] = {"SSFTABLE", "NODE", "PARENT"};

bool validade(String node, String parent) {
  if (!strcmp(node, parent))
      return false;
  if (notexist(columns[0])) {
    Val vals[] = { makeval(NULL, sdt_type),
                   makeval(NULL, sdt_type) };
    sqlCreate(columns[0], vals, 2);
  }
  
  State *state = last_state(node);
  if (state->type != start && state->type != stop) {
    error("unvalid table type");
    freestate(state);
    return false;
  }
  
  int size;
  String *linkables = listlinkables(&size);
  bool valid = false;
  for (int i = 0; i < size; i++) {
    if (!strcmp(linkables[i], node)) {
      valid = true;
      break;
    }
  }
  des_strs(linkables, size);
  freestate(state);
  return valid;
}

bool valid_node(String ssf) {
  Val cols[] = { makeval(columns[1], sdt_type) };
  Result *res = sqlRead(columns[0], cols, 1, 0, 0, 0);
  Row *row = res->table->row;
  for (int i = 0; i < res->table->size; i++) {
    if (!strcmp(ssf, row->val[3])) {
      des_res(res);
      return false;
    }
    row=row->nxt;
  }
  des_res(res);
  return true;
}

void break_branch(String node, String parent) {
  String clause = cat(7, prependType(columns[1], sdt_type), " = ", node, " and ", prependType(columns[2], sdt_type), " = ", parent);
  deleteRecords(columns[0], clause);
}

void break_leaf(String node) {
  String clause = cat(3, prependType(columns[1], sdt_type), " = ", node);
  deleteRecords(columns[0], clause);
}
  

void freessf(SSF *node) {
  if (node->parent)
    free(node->parent);
  int child_num = 0;
  SSF *child = node->children?node->children:NULL;
  while (child) {
    freessf(child);
    child = node->children+(child_num++*8); //null-terminated
  }
  free(node);
}
//append child to null-terminated list.
SSF* append_child(SSF *tree, SSF *child, int child_num) {
  SSF **children = malloc((child_num+2)*sizeof(SSF*));
  for (int i = 0; i < child_num; i++) {
    memcpy(children[i], tree->children+(i*8), sizeof(SSF));
    freessf(children[i]);
  }
  memcpy(children[child_num], child, sizeof(SSF));
  freessf(child);
  children[child_num+1] = NULL;
  return children[0];
}


SSF* read_tree(String node, SSF *tree) {
  if (!node)
    return NULL;
  if (!tree) {
    tree = malloc(sizeof(SSF));
    tree->name = strdup(node);
    tree->parent, tree->children = NULL;
  }
  State *state = last_state(tree->name);
  tree->state = state->type;
  freestate(state);
  String parent;
  String *child;
  int psize = 0;
  int csize = 0;
  int child_num = 0;
  Val cols[] = { makeval(columns[1], sdt_type),
                 makeval(columns[2], sdt_type) };
  Result *res = sqlRead(columns[0], cols, 2, 0, 0, 0);
  Row *row = res->table->row;
  for (int i = 0; i < res->table->size; i++) {
    if (!tree->parent && !strcmp(row->val[2], node)) {
      tree->parent = malloc(sizeof(SSF));
      tree->parent->name = strdup(row->val[3]);
      tree->parent->parent = NULL;
      tree->parent->children = tree;
      read_tree(tree->parent->name, tree->parent);
    }
    else if (!strcmp(row->val[3], node)) {
      SSF *child = malloc(sizeof(SSF));
      child->name = strdup(row->val[2]);
      child->parent = tree;
      child->children = NULL;
      append_child(tree, child, child_num);
      read_tree(child->name, child);
      child_num++;
    }
  }
  des_res(res);
  return tree;
}

void add_branch(String node, String parent) {
  if (!validade(node, parent)) {
    error("givn ssf table unvalid");
    return;
  }
  
  if (!valid_node(node)) {
    Val vals[] = { makeval(node, sdt_type),
                   makeval(parent, sdt_type) };
    sqlInsert(columns,vals, 2);
  }
}
