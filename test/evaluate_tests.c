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

void evaluate_test2(void **state) {
  BOARD * board;
  int score;

  /* P+B vs 4P*/
  board = parse_fen("6k1/4pppp/8/8/8/8/PB6/1K6 w - - 0 1");
  score = evaluate(board);

  assert_true(score > 0);

  free(board);
}

void evaluate_test3(void **state) {
  BOARD * board;
  int score;

  /* 2P+2N vs P+2B*/
  board = parse_fen("6k1/5bbp/8/8/8/8/PNNP4/1K6 b - - 0 1");
  score = evaluate(board);

  assert_true(score > 0);

  free(board);
}

void evaluate_test4(void **state) {
  BOARD * board;
  int score;

  /* 2P+Q vs 3P+N+R*/
  board = parse_fen("5rk1/5ppp/5n2/8/8/8/PP6/1KQ5 b - - 0 1");
  score = evaluate(board);

  assert_true(score < 0);

  free(board);
}

void evaluate_test5(void **state) {
  BOARD * board;
  int score;

  /* 2P+Q vs 3P+N+R*/
  board = parse_fen("5rk1/5ppp/5b2/8/8/8/PP6/1KQ5 b - - 0 1");
  score = evaluate(board);

  assert_true(score < 0);

  free(board);
}

