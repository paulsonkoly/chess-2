#include <stdlib.h>
#include <stdio.h>

#include "perft.h"

#include "move.h"
#include "movegen.h"
#include "moveexec.h"

unsigned long long perft(const BOARD * board, int depth, int print) {
  MOVE moves[100];
  MOVE * moveptr;
  BOARD copy;
  unsigned long long int count = 0;

  if (depth == 0) {
    return 1;
  }

  moveptr = add_moves(board, moves, ALL_MOVES);

  for (MOVE * ptr = moves; ptr != moveptr; ptr++) {
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

  return count;
}
