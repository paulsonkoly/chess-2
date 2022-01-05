#include <stdlib.h>

#include "movelist.h"

#define MAX_MOVES 2048
#define MAX_PLYS  64

/* --------------- ply0 -
 * move0                  <-frame[0]
 * move1
 * move2                  <-next[0]
 * move3
 * ..                     <-free[0]
 * ..
 * --------------- ply1 -
 * ..                     <-frame[1]
 */
static int ply = -1;

static MOVE store[MAX_MOVES];
/* first move of each ply */
static MOVE * frame[MAX_PLYS];
/* the free slot */
static MOVE * alloc;
/* first move to play within ply */
static MOVE * first[MAX_PLYS];

void ml_open_frame() {
  ply++;
  if (ply == 0) {
    alloc = store;
  }
  frame[ply] = alloc;
  first[ply] = NULL;
}

void ml_close_frame() {
  alloc = frame[ply];
  ply--;
}

MOVE * ml_allocate() {
  MOVE * ret = alloc;
  alloc->next = first[ply];
  first[ply] = alloc;
  alloc++;
  return ret;
}

static void heuristic_weights(const MOVE * pv, int depth, const KILLER * killer);
static MOVEVAL capture_value(const MOVE * ptr);
static void sort();

MOVE * ml_sort(const MOVE * pv, int depth, const KILLER * killer) {
  heuristic_weights(pv, depth, killer);
  sort();
  return first[ply];
}

MOVE * ml_first() {
  return first[ply];
}

static void heuristic_weights(const MOVE * pv, int depth, const KILLER * killer) {
  for (MOVE * ptr = frame[ply]; ptr < alloc; ++ptr) {
    if (pv != NULL && MOVE_EQUAL(pv, ptr)) {
      ptr->value = 200;
    } else if (killer != NULL && is_killer(killer, depth, 1, ptr)) {
      ptr->value = 190;
    } else if (killer != NULL && is_killer(killer, depth, 2, ptr)) {
      ptr->value = 180;
    } else {
      ptr->value = capture_value(ptr);
    }
  }
}

static const MOVEVAL piece_values[] = { 0, 10, 30, 32, 50, 90 };

static MOVEVAL capture_value(const MOVE * move) {
  switch (move->capture) {
    case NO_PIECE:
    /* maximum capture is capturing queen with pawn +80 to minimum capture -80 offset by 80 this puts non captures in
     * the middle, but 160 still being smaller than the killers
     */
    return 80;

    case KING:
    /* invalid move, will be ignored by the search, move it to the end */
    return 0;

    default:
      return (piece_values[move->capture] - piece_values[move->piece]) + 90;
  }
}

void sort() {
  MOVEVAL min = 0;
  first[ply] = NULL;

  while (1) {
    MOVEVAL max = 255;
    int any = 0;

    for (MOVE * ptr = frame[ply]; ptr < alloc; ++ptr) {
      if (min < ptr->value && ptr->value < max) {
        max = ptr->value;
        any = 1;
      }
    }

    if (!any) {
      return;
    }

    for (MOVE * ptr = frame[ply]; ptr < alloc; ++ptr) {
      if (ptr->value == max) {
        ptr->next = first[ply];
        first[ply] = ptr;
      }
    }

    min = max;
  }
}
