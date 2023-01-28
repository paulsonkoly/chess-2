#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>

#include <cmocka.h>

#include "attacks_tests.h"

#include "board.h"
#include "attacks.h"

void attacks_test1(void **state) {
  BOARD * board;

  board = initial_board();

  assert_false(checkmate(board));

  free(board);
}

void attacks_test2(void **state) {
  BOARD * board;

  board = parse_fen("6k1/8/8/8/8/8/6PP/3r3K w - - 0 1");

  assert_true(checkmate(board));

  free(board);
}

void attacks_test3(void **state) {
  BOARD * board;
  /* capture the checker */
  board = parse_fen("6k1/8/8/8/8/2N5/6PP/3r3K w - - 0 1");

  assert_false(checkmate(board));

  free(board);
}

void attacks_test4(void **state) {
  BOARD * board;
  /* double check checkmate */
  board = parse_fen("6k1/8/8/7r/8/2N5/5nP1/6BK w - - 0 1");

  assert_true(checkmate(board));

  free(board);
}

void attacks_test5(void **state) {
  BOARD * board;
  /* piece blocks check */
  board = parse_fen("6k1/B7/8/8/8/8/6PP/2r4K w - - 0 1");

  assert_false(checkmate(board));

  free(board);
}

void attacks_test6(void **state) {
  BOARD * board;
  /* double pawn push blocks check */
  board = parse_fen("6k1/8/8/2b5/8/8/3P2PP/5BKR w - - 0 1");

  assert_false(checkmate(board));

  free(board);
}

void attacks_test7(void **state) {
  BOARD * board;
  /* blocking pawns are pinned or blocked */
  board = parse_fen("6k1/8/8/1b6/3PP3/r1PKP3/2PRB3/8 w - - 0 1");

  assert_true(checkmate(board));
  free(board);
}


