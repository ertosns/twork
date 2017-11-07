#include "ssf.h"

String columns[] = {"SSFTABLE", "NODE", "PARENT"};

bool validade(String node, String parent) {
  if (notexist(columns[0])) {
    Val vals[] = { makeval(columns[1], sdt_type),
                   makeval(columns[2], sdt_type) };
    sqlCreate(columns[0], vals, 2);
  }
  if (!strcmp(node, parent))
    return false;

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
  validade(ssf, ssf);
  Val cols[] = { makeval(columns[1], sdt_type) };
  Result *res = sqlRead(columns[0], cols, 1, 0, 0, 0);
  handleRes(res);
  if (!res->table || !res->table->size)
    goto des;
  Row *row = res->table->row;
  for (int i = 0; i < res->table->size; i++) {
    if (!strcmp(ssf, row->val[0])) {
      des_res(res);
      return false;
    }
    row=row->nxt;
  }
 des:
  des_res(res);
  return true;
}

void freessf(SSF *node) {
  if(!node)
    return;
  if (node->parent);
  //freessf(node->parent);
  int i = 0;
  for (; node->children && node->children[i]; i++)
    freessf(node->children[i]);
  //free(node->name); fails
  //free(node); fails for some reason?!
}
//append child to null-terminated list.
void append_child(SSF *tree, SSF *child, int child_num) {
  SSF **children = malloc((child_num+2)*sizeof(SSF*));
  for (int i = 0; i < child_num; i++) {
    memcpy(children[i], tree->children[i], sizeof(SSF));
    freessf(tree->children[i]);
  }
  children[child_num] = child;
  children[child_num+1] = NULL;
  tree->children = children;
}

SSF* read_tree(String node, SSF *tree) {
  if (!node)
    return NULL;
  if (!tree) {
    tree = malloc(sizeof(SSF));
    tree->name = strdup(node);
    tree->parent = NULL, tree->children = NULL;
  }
  State *state = last_state(tree->name);
  tree->state = state->type;
  freestate(state);
  Val cols[] = { makeval(columns[1], sdt_type),
                 makeval(columns[2], sdt_type) };
  Result *res = sqlRead(columns[0], cols, 2, 0, 0, 0);
  handleRes(res);
  if (!res->table)
    goto des;
  Row *row = res->table->row;
  SSF *child;
  int c = 0;
  for (int i = 0; i < res->table->size; i++) {
    if (!tree->parent && !strcmp(row->val[0], node)) {
      tree->parent = malloc(sizeof(SSF));
      tree->parent->name = strdup(row->val[1]);
      tree->parent->parent = NULL;
      tree->parent->children = malloc(sizeof(SSF));
      tree->parent->children[0] = tree;
      read_tree(tree->parent->name, tree->parent);
    } else if (!strcmp(row->val[1], node)) {
      child = malloc(sizeof(SSF));
      child->name = strdup(row->val[0]);
      child->parent = tree;
      child->children = NULL;
      append_child(tree, child, c);
      read_tree(child->name, tree->children[c++]);
    }
  }
 des:
  des_res(res);
  return tree;
}

void add_branch(String node, String parent) {
  if (!validade(node, parent)) {
    error("givn ssf table unvalid");
    return;
  }
  
  if (valid_node(node)) {
    Val vals[] = { makeval(node, sdt_type),
                   makeval(parent, sdt_type) };
    sqlInsert(columns,vals, 2);
  }
}

void break_branch(String node, String parent) {
  String clause = cat(8, prependType(columns[1], sdt_type), " = '", node, "' and ", prependType(columns[2], sdt_type), " = '", parent, "'");
  deleteRecords(columns[0], clause);
}

void break_leaf(String node) {
  String clause = cat(4, prependType(columns[1], sdt_type), " = '", node, "'");
  deleteRecords(columns[0], clause);
}
