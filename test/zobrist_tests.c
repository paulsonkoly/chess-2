#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>

#include <cmocka.h>

#include "zobrist_tests.h"

#include "attacks.h"
#include "board.h"

void zobrist_test1(void **state) {
  BOARD * board;

  initialize_magic();
  initialize_hash();

  board = parse_fen("k7/4p3/8/5P2/8/8/8/4K2R w K - 0 1");
  play_uci_moves(board, "e1g1 e7e5 f5e6 a8a7 e6e7 a7a6 e7e8q");

  assert_int_equal(board->history[board->halfmovecnt], calculate_hash(board));

  free(board);
}

void zobrist_test2(void **state) {
  BOARD * board;

  initialize_magic();
  initialize_hash();

  board = parse_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
  play_uci_moves(board, "e1e2 e8e7 e2e1 e7e8");

  assert_int_equal(board->history[board->halfmovecnt], board->history[0]);

  free(board);
}
