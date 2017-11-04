#include "pivot.h"

String column[] = {"PIVOT_COLUMN", "PIVOT", "STATE"};

void valid_pivot() {
  if (notexist(column[0])) {
    Val vals[] = { makeval(NULL, sdt_type),
                   makeval(NULL, sdt_number) };
    sqlCreate(column[0], vals, 2);
  }
}

void freepiv(Pivot *piv) {
  free(piv->start_stamp);
  if (piv->stop_stamp)
    free(piv->stop_stamp);
  free(piv);
}

int last_pivot_state(String pivot) {
  Pivot *piv = get_pivot(pivot);
  int state =  piv->stop_stamp?stop:start;
  freepiv(piv);
  return state;
}

void start_pivot(String pivot) {
  del_pivot(pivot);
  Val vals[] = { makeval(pivot, sdt_type),
                 makeval(itos(start), sdt_number) };
  sqlInsert(column, vals, 2);
}

void stop_pivot(String pivot) {
  if (last_pivot_state(pivot)==stop) {
    error("pivot already stopped");
    return;
  }
  Val vals[] = { makeval(pivot, sdt_type),
                 makeval(itos(stop), sdt_number) };
  sqlInsert(column, vals, 2);
}

void del_pivot(String piv) {
  String clause = cat(3, prependType(column[1], sdt_type), " = ",  piv);
  deleteRecords(column[0], clause);
}

Pivot* get_pivot(String pivot) {
  Val cols[] = { makeval(column[1], sdt_type),
                 makeval(column[2], sdt_number) };
  String pivot_clause = cat(3, prependType(column[1], sdt_type), "=", pivot);
  Result *pivot_res = sqlRead(column[0], cols, 2, 2, 0, pivot_clause);
  handleRes(pivot_res);
  Row *row = pivot_res->table->row;
  Pivot *piv = malloc(sizeof(Pivot));
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
  Val cols[] = { makeval(column[1], sdt_type) };
  String piv_clause = cat(3, prependType(column[2], sdt_number), " = ", itos(start));
  Result *res = sqlRead(column[0], cols, 1, 0, 0, piv_clause);
  handleRes(res);
  if (res->table->size<1)
    return NULL;
  Row *row = res->table->row;
  do {
    append(pivots, *size, row->val[2]);
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
      append(open, pcount++, piv->name);
    freepiv(piv);
  }
  des_strs(pivots, *size);
  return open;
}

Cumulate* cumulate_pivot(Pivot *piv, int *size) {
  if (!piv->stop_stamp)
    piv->stop_stamp = gmtime((time_t*)time(NULL));
  String *linkables = listlinkables(size);
  Cumulate *cumulates = malloc(*size*sizeof(Cumulate));
  float tota;
  int type;
  for (int i = 0; i<*size; i++) {
    tota = cumulate(linkables[i], piv->start_stamp, piv->stop_stamp, &type);
    cumulates[i].name = strdup(linkables[i]);
    cumulates[i].tota = tota;
    cumulates[i].type = type;
  }
  des_strs(linkables, *size);
  return cumulates;
}

String cumulate_pivot_str(Pivot *piv) {
  String cumulatestr = NULL;
  int size;
  Cumulate *cumulates = cumulate_pivot(piv, &size);
  String coef = cumulates[0].type==start?"hours\n":"\n";
  for (int i = 0; i<size; i++) {
    cumulatestr = cat(5, cumulatestr, cumulates[i].name, ": ", ftos(cumulates[i].tota), coef);
    free(cumulates+i*8);
  }
  return cumulatestr;
}
