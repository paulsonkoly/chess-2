#ifndef _KILLER_H_
#define _KILLER_H_

#include "move.h"

#define KILLER_MAX_DEPTH 30

typedef struct _KILLER_ {
  MOVE moves[KILLER_MAX_DEPTH][2];
  int valid[KILLER_MAX_DEPTH][2];
} KILLER;

void reset_killer(KILLER * killer);

void save_killer(KILLER * killer, int depth, MOVE * move);

const MOVE * killer_get_move(const KILLER * killer, int ply, int lane);

/* int is_killer(const KILLER * killer, int depth, int second, const MOVE * move); */

#endif /* ifndef _KILLER_H_ */
