#ifndef __TRANSPOSITION_H__
#define __TRANSPOSITION_H__

#include "zobrist.h"

typedef enum { TT_TYPE_EXACT = 1, TT_TYPE_LOWER = 2, TT_TYPE_UPPER = 3 } TT_TYPE;

typedef struct __TT_RESULT__ {
  HASH hash;
  TT_TYPE type;
  int score;
  int depth;
} TT_RESULT;

/* Implements a 2 lane transposition table, with lane 1 greater depth and lane 2 always replacement policies. */

void initialize_tt();
void tt_reset();
void tt_free();
void tt_info();
const TT_RESULT * tt_probe(HASH hash, int depth);
void tt_insert_or_replace(HASH hash, int depth, int score, TT_TYPE type);

#endif /* ifndef __TRANSPOSITION_H__ */
