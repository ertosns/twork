#include "pivot.h"

String column[] = {"PIVOT", "PIVOT_NAME", "STATE"};

void valid_pivot() {
  if (notexist(column[0])) {
    Val vals[] = { makeval(column[1], sdt_type),
                   makeval(column[2], sdt_number) };
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
  valid_pivot();
  String clause = cat(4, prependType(column[1], sdt_type), " = '",  piv, "'");
  deleteRecords(column[0], clause);
}

Pivot* get_pivot(String pivot) {
  String pivot_clause = cat(4, prependType(column[1], sdt_type), "='", pivot, "'");
  Result *pivot_res = sqlRead(column[0], 0, 0, 2, 0, pivot_clause);
  handleRes(pivot_res);
  if (!pivot_res->table->size)
    return NULL;
  Row *row = pivot_res->table->row;
  Pivot *piv = malloc(sizeof(Pivot));
  piv->name = strdup(pivot);
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
  valid_pivot();
  *size = 0;
  String *pivots = NULL;
  Val cols[] = {makeval(column[1], sdt_type)};
  String piv_clause = cat(4, prependType(column[2], sdt_number), " = '", itos(start), "'");
  Result *res = sqlRead(column[0], cols, 1, 0, 0, piv_clause);
  handleRes(res);
  if (!res->table || res->table->size<1)
    return NULL;
  Row *row = res->table->row;
  for (int i = 0; i < res->table->size; i++) {
    pivots = append(pivots, *size, row->val[0]);
    row = row->nxt;
    (*size)++;
  }
  des_res(res);
  return pivots;
}

String* list_current_pivots(int *size) {
  String *open = NULL;
  int pcount = 0;
  String *pivots = list_pivots(size);
  Pivot *piv;
  for (int i = 0; i < *size; i++) {
    piv = get_pivot(pivots[i]);
    if (!piv->stop_stamp)
      open = append(open, pcount++, piv->name);
    freepiv(piv);
  }
  des_strs(pivots, *size);
  return open;
}

Cumulate** cumulate_pivot(Pivot *piv, int *size) {
  if (!piv->stop_stamp)
    piv->stop_stamp = current_tm();
  String *linkables = listlinkables(size);
  Cumulate **cumulates = malloc(*size*sizeof(Cumulate*));
  for (int i = 0; i < *size; i++)
    cumulates[i] = malloc(sizeof(Cumulate));
  float tota;
  int type;
  for (int i = 0; i<*size; i++) {
    tota = cumulate(linkables[i], piv->start_stamp, piv->stop_stamp, &type);
    cumulates[i]->name = strdup(linkables[i]);
    cumulates[i]->tota = tota;
    cumulates[i]->type = type;
  }
  des_strs(linkables, *size);
  return cumulates;
}

String cumulate_pivot_str(Pivot *piv) {
  String cumulatestr = NULL;
  int size;
  Cumulate **cumulates = cumulate_pivot(piv, &size);
  String coef = cumulates[0]->type==start?"hours\n":"\n";
  for (int i = 0; i<size; i++) {
    cumulatestr = cat(5, cumulatestr, cumulates[i]->name, ": ", ftos(cumulates[i]->tota/3600), coef);
    free(cumulates[i]);
  }
  return cumulatestr;
}
