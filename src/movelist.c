#include "movelist.h"

#include <stdlib.h>
#include <limits.h>

#include "attacks.h"
#include "moveexec.h"
#include "chess.h"
#include "see.h"

#define MAX_MOVES 2048


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

static void heuristic_weights(BOARD * board, const MOVE * pv, int depth, const KILLER * killer);
static void forcing_weights(BOARD * board);
static void sort();

MOVE * ml_sort(BOARD * board, const MOVE * pv, int depth, const KILLER * killer) {
  heuristic_weights(board, pv, depth, killer);
  sort();
  return first[ply];
}

MOVE * ml_first() {
  return first[ply];
}

MOVE * ml_forcing(BOARD * board) {
  forcing_weights(board);
  sort();
  return first[ply];
}

static void heuristic_weights(BOARD* board, const MOVE * pv, int depth, const KILLER * killer) {
  for (MOVE * ptr = frame[ply]; ptr < alloc; ++ptr) {
    if (pv != NULL && MOVE_EQUAL(pv, ptr)) {
      ptr->value = 10100;
    } else if (killer != NULL && is_killer(killer, depth, 1, ptr)) {
      ptr->value = 10090;
    } else if (killer != NULL && is_killer(killer, depth, 2, ptr)) {
      ptr->value = 10080;
    } else {
      ptr->value = see(board, ptr) + 1000;
    }
  }
}

static void forcing_weights(BOARD * board) {
  SQUARE king = __builtin_ctzll(board->kings & COLOUR_BB(board, board->next ^ 1));

  for (MOVE * ptr = frame[ply]; ptr < alloc; ++ptr) {
    if (ptr->special & CAPTURED_MOVE_MASK) {
      ptr->value = see(board, ptr) + 1000;
    } else {
      if (move_attacks_sq(board, ptr, king)) {
        ptr->value = 1000;
      }
      else {
        ptr->value = 0; /* remove the move - not forcing */
      }
    }
  }
}

void sort() {
  MOVEVAL min = 0;
  first[ply] = NULL;

  while (1) {
    MOVEVAL max = INT_MAX;
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
