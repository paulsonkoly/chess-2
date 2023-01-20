#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>

#include <cmocka.h>

#include "mat_tables_tests.h"

#include "mat_tables.h"
#include "board.h"

void mat_tables_test1(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* B */
  board = parse_fen("6k1/8/8/8/8/8/8/1K3B2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test2(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* N */
  board = parse_fen("6k1/8/8/8/8/8/8/1K3N2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test3(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* B vs N */
  board = parse_fen("2b3k1/8/8/8/8/8/8/1K3N2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test4(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* R */
  board = parse_fen("6k1/8/8/8/8/8/8/1K3R2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_false(e->flags & DRAWN);
  assert_true(e->flags & W_CHECKMATING);

  free(board);
}

void mat_tables_test5(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* Q */
  board = parse_fen("6k1/8/8/8/8/8/8/1K3Q2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_false(e->flags & DRAWN);
  assert_true(e->flags & W_CHECKMATING);

  free(board);
}

void mat_tables_test6(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* B+N vs N */
  board = parse_fen("4n1k1/8/8/8/8/8/8/K1NB4 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test7(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* R vs B */
  board = parse_fen("4b1k1/8/8/8/8/8/8/KR6 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test8(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* R+B vs R */
  board = parse_fen("4r1k1/8/8/8/8/8/8/KRB5 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test9(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* R+N vs R */
  board = parse_fen("4r1k1/8/8/8/8/8/8/KRN5 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test10(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* Q vs 2B */
  board = parse_fen("4bbk1/8/8/8/8/8/8/K1Q5 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test11(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* Q vs 2N */
  board = parse_fen("4nnk1/8/8/8/8/8/8/K1Q5 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test12(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* Q+N vs Q */
  board = parse_fen("4qnk1/8/8/8/8/8/8/K1Q5 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test13(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* Q+B vs Q */
  board = parse_fen("4qbk1/8/8/8/8/8/8/K1Q5 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test14(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* N vs P */
  board = parse_fen("6k1/6p1/8/8/8/8/8/K1N5 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_int_equal(e->value, -1 * KNIGHT_V);

  free(board);
}

void mat_tables_test15(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* B + N */
  board = parse_fen("6k1/8/8/8/8/8/8/BK3N2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & W_CHECKMATING);
  assert_true(e->flags & BN_MATE_DSQ);

  free(board);
}

void mat_tables_test16(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* opposite colour bishop endgame - 1 pawn diff */
  board = parse_fen("5bk1/5pp1/8/8/8/8/4KPB1/8 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_int_equal(e->value, PAWN_V);

  free(board);
}

void mat_tables_test17(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* opposite colour bishop endgame - 2 pawn diff */
  board = parse_fen("5bk1/5ppp/8/8/8/8/4KPB1/8 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_int_equal(e->value, 80);

  free(board);
}

void mat_tables_test18(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* bishop pair vs 2 N */
  board = parse_fen("r4rk1/pppp1ppp/2n2n2/8/2B5/1P6/PBPP1PPP/R4RK1 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_int_equal(e->value, 40);

  free(board);
}

void mat_tables_test19(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;
  /* bishop pair vs N+B */
  board = parse_fen("r4rk1/pppp1ppp/2b2n2/8/2B5/1P6/PBPP1PPP/R4RK1 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_int_equal(e->value, 30);

  free(board);
}

void mat_tables_test20(void **state) {
  BOARD * board[2];
  const MAT_TABLE_ENTRY * e[2];
  int i;

  /* trade down: pawn advantage is greater with less pieces */
  board[0] = parse_fen("6k1/5npp/8/4N3/8/8/5PPP/6K1 w - - 0 1");
  board[1] = parse_fen("6k1/6pp/8/8/8/8/5PPP/6K1 w - - 0 1");
  for (i = 0; i < 2; ++i) {
    e[i] = get_mat_table_entry(board[i]);
  }

  assert_true(e[1]->value > e[0]->value);

  for (i = 0; i < 2; ++i) {
    free(board[i]);
  }
}

