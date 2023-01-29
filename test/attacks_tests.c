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

  board = parse_fen("1k6/8/8/8/8/8/5PPP/2r3K1 w - - 0 1");

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

void attacks_test8(void **state) {
  BOARD * board;
  /* checkmate prevented by en-passant capture */
  board = parse_fen("6k1/8/1p6/2pPP3/2PKP3/2PRB3/8/8 w - c6 0 2");

  assert_false(checkmate(board));
  free(board);
}

void attacks_test9(void **state) {
  BOARD * board;
  /* checkmate prevented by normal pawn capture */
  board = parse_fen("6k1/8/1p6/2pPP3/1PPKP3/2NRB3/8/8 w - - 0 1");

  assert_false(checkmate(board));
  free(board);
}

void attacks_test10(void **state) {
  BOARD * board;
  /* checkmate prevented by normal pawn capture */
  board = initial_board();

  assert_false(stalemate(board));
  free(board);
}

void attacks_test11(void **state) {
  BOARD * board;
  /* classic pawn ending stalemate*/
  board = parse_fen("8/8/8/8/8/6k1/6p1/6K1 w - - 0 1");

  assert_true(stalemate(board));
  free(board);
}

void attacks_test12(void **state) {
  BOARD * board;
  /* pinned queen */
  board = parse_fen("8/8/8/8/8/4p3/4P1k1/1r1QK3 w - - 0 1");

  assert_false(stalemate(board));
  free(board);
}

void attacks_test13(void **state) {
  BOARD * board;
  /* pinned knight */
  board = parse_fen("8/8/8/8/8/4p3/4P1k1/1r1NK3 w - - 0 1");

  assert_true(stalemate(board));
  free(board);
}

void attacks_test14(void **state) {
  BOARD * board;
  /* pinned bishop */
  board = parse_fen("8/8/8/8/8/4p3/4P1k1/1r1BK3 w - - 0 1");

  assert_true(stalemate(board));
  free(board);
}

void attacks_test15(void **state) {
  BOARD * board;
  /* pawn capture */
  board = parse_fen("8/8/8/3p4/2P5/4p3/4P1k1/1r1BK3 w - - 0 1");

  assert_false(stalemate(board));
  free(board);
}

void attacks_test16(void **state) {
  BOARD * board;
  /* pinned pawn */
  board = parse_fen("8/8/8/b7/3p4/2P1p3/4P1k1/1r1BK3 w - - 0 1");

  assert_true(stalemate(board));
  free(board);
}

void attacks_test17(void **state) {
  BOARD * board;
  /* pinned pawn can capture */
  board = parse_fen("8/8/8/8/1b6/2P1p3/4P1k1/1r1BK3 w - - 0 1");

  assert_false(stalemate(board));
  free(board);
}

void attacks_test18(void **state) {
  BOARD * board;
  /* en passant capture is only move */
  board = parse_fen("8/8/5p2/4pP2/8/4p3/4P1k1/1r1BK3 w - e6 0 2");

  assert_false(stalemate(board));
  free(board);
}

void attacks_test19(void **state) {
  BOARD * board;
  /* en passant capture is pinned */
  board = parse_fen("1kb4q/6p1/3p2P1/r2Pp1K1/r7/8/8/8 w - e6 0 2");

  assert_true(stalemate(board));
  free(board);
}

void attacks_test20(void **state) {
  BOARD * board;
  /* a tricky checkmate */
  board = parse_fen("rnbq2kr/pp1PbQpp/2p5/8/2B5/8/PPP1NnPP/RNB1K2R b KQ - 0 1");

  assert_true(checkmate(board));
  free(board);
}



