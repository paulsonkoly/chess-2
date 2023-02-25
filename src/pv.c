#include <stdlib.h>
#include <assert.h>

#include "chess.h"
#include "pv.h"

#define DEBUG_PV 1

typedef struct _PV_ {
  int count;
  MOVE store[MAX_PLIES];
} PV;

PV * pv_init(void) {
  PV * ret;
  if (NULL != (ret = malloc(sizeof(PV)))) {
    ret->count = 0;
  }

  return ret;
}

void pv_destroy(PV * pv) {
  free(pv);
}

void pv_reset(PV * pv) {
  pv->count = 0;
}

void pv_swap(PV** pv1, PV** pv2) {
  PV * tmp = * pv1;
  *pv1 = *pv2;
  *pv2 = tmp;
}

void pv_insert(PV * pv, const MOVE * move, int offset) {
#if defined(DEBUG_PV)
  assert(offset < MAX_PLIES);
#endif
  pv->store[offset] = *move;
  pv->count = pv->count + 1;
}


const MOVE * pv_get_move(const PV * pv, int offset) {
  return (pv->count > offset ? pv->store + offset : NULL);
}

int pv_count(const PV* pv) { return pv->count; }
