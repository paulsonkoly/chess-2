#include <string.h>

#include "killer.h"


void reset_killer(KILLER * killer) {
  for (int i = 0; i < KILLER_MAX_DEPTH; ++i) {
    killer->valid[i][0] = 0;
    killer->valid[i][1] = 0;
  }
}

void save_killer(KILLER * killer, int depth, MOVE * move) {
  int index = 0;

  if (killer->valid[depth][0]) {
    index = 1;
  }

  killer->valid[depth][index] = 1;
  memcpy(&killer->moves[depth][index], move, sizeof(MOVE));
}

void apply_killers(const KILLER * killer, MOVE * moves, MOVE * last, int depth) {
  for (int i = 0; i < 2; ++i) {
    if (killer->valid[depth][i]) {
      for (MOVE * ptr = moves; ptr != last; ptr++) {
        if (ptr->from == killer->moves[depth][i].from && ptr->to == killer->moves[depth][i].to) {
          MOVE tmp;

          tmp = *ptr;
          *ptr = *moves;
          *moves = tmp;

          break;
        }
      }
    }
  }
}
