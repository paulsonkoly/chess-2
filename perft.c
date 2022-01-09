#include "perft.h"

#include <stdio.h>

#include "attacks.h"
#include "move.h"
#include "movegen.h"
#include "moveexec.h"
#include "movelist.h"

unsigned long long perft(BOARD * board, int depth, int print) {
  unsigned long long int count = 0;

  if (depth == 0) {
    return 1;
  }

  ml_open_frame();

  add_moves(board, ALL_MOVES);

  for (MOVE * ptr = ml_first(); ptr != NULL; ptr = ptr->next) {
    unsigned long long int current;

    execute_move(board, ptr);

    if (in_check(board, 1 - board->next)) {
      undo_move(board, ptr);

      continue;
    }

    if (print) print_fen(board);

    current = perft(board, depth - 1, 0);

    undo_move(board, ptr);

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
