#include <pivot.h>

const String tbl[] = {"PIVOT_TBL",  "PIVOT", "DESCRIPTION", "STATE"};

//todo fix col in sqlread
void valid_pivot() {
  if (notexists(PIVOT_TBL)) {
    Val vals[] = { makeval(NULL, std_type),
                   makeval(NULL, std_type),
                   makeval(NULL, std_number)};
    sqlCreate(tbl, vals, 3));
}
}

void start_pivot(String pivot, String des) {
  if (last_pivot_state(pivot)==start)
    stop_pivot(String pivot);  
  Val vals[] = { makeval(pivot, std_type),
                 makeval(des, std_type),
                 makeval(start, std_number) };
  sqlInsert(tbl, vals, 3);
}

void stop_pivot(String pivot) {
  if (last_pivot_state(pivot)==stop) {
    error("pivot already stopped");
    return;
  }
  Val vals[] = { makeval(pivot, std_type),
                 makeval(NULL, std_type),
                 makeval(stop, std_number) };
  sqlInsert(tbl, vals, 3);
}

void del_pivot(Pivot *piv) {
  String clausse = cat(4, "ROWID == ",  itoa(piv->start_rowid), " OR ROWID == ", itoa(piv->stop_rowid));
  deleteRecords(tbl[0], clause);
}

Pivot* get_pivot(String pivot) {
  Val cols[] = { makeval(tbl[1], std_type),
                 makeval(tbl[2], std_type),
                 makeval(tbl[3], std_number) };
  String last_start_clause = cat(7, tbl[1], "=", pivot, " and ",  tbl[3], "=", itoa(start));
  Result *last_start_res = sqlRead(tbl[0], cols, 3, 1, 0, last_start_clause);
  handleRes(last_start_res);
  if (last_start_res->table->size < 1)
    return NULL;
  Pivot *piv = malloc(sizeof);
  piv->rowid = last_start_id;
  piv->name = strdup(pivot);
  int last_start_id = atoi(last_start_res->table->row->val);
  des_res(last_start_res);
  String pivot_clause = cat(7, tbl[1], "=", pivot, " and ", ROWID, ">=", last_start_id);
  Result *pivot_res = sqlRead(tbl[0], cols, 3, 2, 0, pivot_clause);
  handleRes(pivot_res);
  //start state.
  Row *row = pivot_res->table->row;
  piv->start_stamp = ts2tm(row->val[1]);
  piv->des = strdup(row->val[3]);
  if (pivot_res->table->size == 1) {
    piv->stop_rowid = -1;
    piv->stop_stamp = NULL;
  } else  {
    row = row->nxt;
    piv->stop_stamp = ts2tm(row->val[1]);
  }
  des_res(pivot_res);
  return piv;
}

String* list_pivots(int *size) {
  *size = 0;
  String *pivots = NULL;
  Val cols[] = { makeval(tbl[1], std_type),
                 makeval(tbl[2], std_type),
                 makeval(tbl[3], std_number) };
  int groupby[] = {1];
  String opivot = NULL;
  Result *res = sqlReadGrouped(tbl[0], cols, 3, 1, groupby, NULL, 0, 0, NULL);
  handleRes(res);
  if (res->table->size<1)
    return NULL;
  Row *row = res->table->row;
  do {
    if (!strcmp(opivot, row->val[2]))
      continue;
    pivots = append(pivots, *size, row->val[2]);
    (*size)++;
  } while (row = row->nxt);
  des_res(res);
  return pivots;
}

Cumulate* cumulate_pivot(Pivot *piv, int *size) {
  if (!piv->stop_stamp) {
    piv->stop_stamp = current_tm();
  }
  String *linkables = listlinkables(size);
  Cumulate *cumulates = malloc(size*sizeof(Cumulate));
  float tota;
  int type;
  for (int i = 0; i<size; i++) {
    tota = cumulate(linkables[i], piv->start_stamp, piv->stop_stamp, &type);
    piv[i].name = strdup(linkables[i]);
    piv[i].tota = tota;
    piv[i].type = type;
  }
  return cumulates;
}

String cumulate_pivot_str(Pivot *piv) {
  String cumulatestr = NULL;
  int size;
  Cumulate *cumulates = cumulate_pivot(piv, &size);
  Cumulate *cumulate;
  String coef = cumulates[0]->type==start?"hours\n":"\n";
  h
  for (int i = 0; i<*size; i++) {
    cumulate = cumulates[i];
    cumulatestr = cat(5, cumulatestr, cumulate.name, ": ", ftos(cumulate.tota), coef);
  }
  return cumulatestr;
}

