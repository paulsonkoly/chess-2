#include <string.h>

#include "killer.h"


void reset_killer(KILLER * killer) {
  for (int i = 0; i < KILLER_MAX_DEPTH; ++i) {
    killer->valid[i][0] = 0;
    killer->valid[i][1] = 0;
  }
}

void save_killer(KILLER * killer, int depth, MOVE * move) {
  /* ix 0: always replace ix 1: never replace */
  int index = killer->valid[depth][1] ^ 1;

  killer->valid[depth][index] = 1;
  memcpy(&killer->moves[depth][index], move, sizeof(MOVE));
}

const MOVE * killer_get_move(const KILLER * killer, int ply, int lane) {
  if (killer->valid[ply][lane]) {
    return & killer->moves[ply][lane];
  }
  return NULL;
}

