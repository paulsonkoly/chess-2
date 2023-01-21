#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>

#include <cmocka.h>

#include "evaluate_tests.h"

#include "evaluate.h"
#include "board.h"

void evaluate_test1(void **state) {
  BOARD * board;
  int score;

  /* P+N vs 4P*/
  board = parse_fen("6k1/4pppp/8/8/8/8/PN6/1K6 w - - 0 1");
  score = evaluate(board);

  assert_true(score > 0);

  free(board);
}
