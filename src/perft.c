#include "perft.h"

#include <stdio.h>

#include "attacks.h"
#include "move.h"
#include "movegen.h"
#include "moveexec.h"

unsigned long long perft(BOARD * board, int ply, int depth, int print) {
  unsigned long long int count = 0;
  int first = 1;
  MOVE * move;

  if (depth == 0) {
    return 1;
  }

  while ((move = moves(board, ply, NULL, NULL, MOVEGEN_NORMAL, first))) {
    unsigned long long int current;
    first = 0;

    execute_move(board, move);

    if (in_check(board, 1 - board->next)) {
      undo_move(board, move);

      continue;
    }

    if (print) print_fen(board);

    current = perft(board, ply + 1, depth - 1, 0);

    undo_move(board, move);

    if (print) {
      print_move(move);
      printf(" %lld\n", current);
    }

    count += current;
  }

  if (print) {
    printf(" %lld\n", count);
  }

  return count;
}
