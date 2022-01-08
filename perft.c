#include "perft.h"

#include <stdio.h>

#include "attacks.h"
#include "move.h"
#include "movegen.h"
#include "moveexec.h"
#include "movelist.h"

unsigned long long perft(const BOARD * board, int depth, int print) {
  BOARD copy;
  unsigned long long int count = 0;

  if (depth == 0) {
    return 1;
  }

  ml_open_frame();

  add_moves(board, ALL_MOVES);

  for (MOVE * ptr = ml_first(); ptr != NULL; ptr = ptr->next) {
    copy = *board;
    unsigned long long int current;

    execute_move(&copy, ptr);

    if (in_check(&copy, 1 - copy.next)) {
      continue;
    }

    if (print) print_fen(&copy);

    current = perft(&copy, depth - 1, 0);

    if (print) {
      print_move(ptr);
      printf(" %lld\n", current);
    }

    count += current;
  }

  ml_close_frame();

  if (print) {
    printf(" %lld\n", count);
  }

  return count;
}
