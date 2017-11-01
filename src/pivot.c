#include <pivot.h>
#define SSF

const String tbl[] = {"PIVOT_TBL", "PIVOT", "STATE"};

void valid_pivot() {
  if (notexists(PIVOT_TBL)) {
    Val vals[] = { makeval(NULL, std_type),
                   makeval(NULL, std_number) };
    sqlCreate(tbl, vals, 2);
  }
}

void freepiv(Pivot *piv) {
  free(piv->start_stamp);
  if (piv->stop_stamp)
    free(piv->stop_stamp);
  free(piv);
}

int last_pivot_state(String pivot) {
  Pivot *piv == get_pivot(pivot);
  int state =  piv->stop_stamp?stop:start;
  freepiv(piv);
  return state;
}

void start_pivot(String pivot) {
  del_pivot(pivot);
  Val vals[] = { makeval(pivot, std_type),
                 makeval(start, std_number) };
  sqlInsert(tbl, vals, 2);
}

void stop_pivot(String pivot) {
  if (last_pivot_state(pivot)==stop) {
    error("pivot already stopped");
    return;
  }
  Val vals[] = { makeval(pivot, std_type),
                 makeval(stop, std_number) };
  sqlInsert(tbl, vals, 2);
}

void del_pivot(String piv) {
  String clausse = cat(3, prependType(tbl[1], std_type), " = ",  piv);
  deleteRecords(tbl[0], clause);
}

Pivot* get_pivot(String pivot) {
  Val cols[] = { makeval(tbl[1], std_type),,
                 makeval(tbl[2], std_number) };
  String pivot_clause = cat(3, prependType(tbl[1], std_type), "=", pivot);
  Result *pivot_res = sqlRead(tbl[0], cols, 2, 2, 0, pivot_clause);
  handleRes(pivot_res);
  Row *row = pivot_res->table->row;
  piv->start_stamp = ts2tm(row->val[1]);
  if (pivot_res->table->size == 1) {
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
  Val cols[] = { makeval(tbl[1], std_type) };
  int groupby[] = {1];
  String piv_clause = cat(3, prependType(tbl[2], std_number), " = ", itoa(start));
  Result *res = sqlRead(tbl[0], cols, 1, 0, 0, piv_clause);
  handleRes(res);
  if (res->table->size<1)
    return NULL;
  Row *row = res->table->row;
  do {
    pivots = append(pivots, *size, row->val[2]);
    (*size)++;
  } while (row = row->nxt);
  des_res(res);
  return pivots;
}

String* list_current_pivots(int *size) {
  String *open = NULL;
  int pcount = 0;
  String *pivots = list_pivots(size);
  for (int i = 0; i < *size; i++) {
    Pivot *piv = get_pivot(pivots[i]);
    if (!piv->stop_stamp)
      pivots = append(pivots, piv->name, pcount++);
    freepiv(piv);
  }
  free_ptr(pivots);
  return open;
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
  String coef = cumulates[0]->type==start?"hours\n":"\n";
  h
  for (int i = 0; i<*size; i++) {
    cumulatestr = cat(5, cumulatestr, cumulate[i].name, ": ", ftos(cumulate[i].tota), coef);
  }
  free(cumulates);
  return cumulatestr;
}
