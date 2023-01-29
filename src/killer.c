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

const MOVE * killer_get_move(const KILLER * killer, int ply, int lane) {
  if (killer->valid[ply][lane]) {
    return & killer->moves[ply][lane];
  }
  return NULL;
}

/* int is_killer(const KILLER * killer, int depth, int bank, const MOVE * move) { */
/*   return killer->valid[depth][bank] && MOVE_EQUAL(& killer->moves[depth][bank], move); */
/* } */

