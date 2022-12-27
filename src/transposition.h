#ifndef __TRANSPOSITION_H__
#define __TRANSPOSITION_H__

#include "zobrist.h"

typedef struct __TT_RESULT__ {
  HASH hash;
  int score;
  int depth;
} TT_RESULT;

void initialize_tt();
void tt_reset();
void tt_free();
void tt_info();
const TT_RESULT * tt_probe(HASH hash, int depth);
void tt_insert_or_replace(HASH hash, int depth, int score);

#endif /* ifndef __TRANSPOSITION_H__ */
