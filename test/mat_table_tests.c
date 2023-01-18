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

  board = parse_fen("6k1/8/8/8/8/8/8/1K3B2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test2(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;

  board = parse_fen("6k1/8/8/8/8/8/8/1K3N2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_true(e->flags & DRAWN);

  free(board);
}

void mat_tables_test3(void **state) {
  BOARD * board;
  const MAT_TABLE_ENTRY * e;

  board = parse_fen("6k1/8/8/8/8/8/8/1K3R2 w - - 0 1");
  e = get_mat_table_entry(board);

  assert_false(e->flags & DRAWN);

  free(board);
}

