#include "movelist.h"

#include <assert.h>
#include <stdlib.h>

#include "chess.h"

/* allocated ply count */
static int ply = 0;

static MOVE store[MAX_MOVES];
/* first move of each ply */
static MOVE * frame[MAX_PLIES];
/* the free slot */
static MOVE * alloc;

void ml_open_frame(void) {
  if (ply == 0) {
    alloc = store;
  }
  frame[ply] = alloc;
  assert(ply < MAX_PLIES);
  ply++;
}

void ml_close_frame(void) {
  assert(ply > 0);
  alloc = frame[--ply];
}

MOVE * ml_first(void) {
  assert(ply > 0);
  return frame[ply - 1];
}

MOVE * ml_last(void) {
  return alloc;
}

MOVE * ml_allocate(void) {
  return alloc++;
}

