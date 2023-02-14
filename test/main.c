#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#include "mat_tables.h"
#include "board.h"
#include "attacks.h"
#include "perft.h"
#include "movelist.h"
#include "movegen.h"
#include "see.h"

#include "attacks_tests.h"
#include "uci_tests.h"
#include "pawns_tests.h"
#include "zobrist_tests.h"
#include "mat_tables_tests.h"
#include "evaluate_tests.h"

int stopped = 0;

static void perft_unit_test1(void **state) {
  BOARD * b = initial_board();

  assert_int_equal(20, perft(b, 0, 1, 0));
}

static void perft_unit_test2(void **state) {
  BOARD * b = initial_board();

  assert_int_equal(400, perft(b, 0, 2, 0));
}

static void perft_unit_test3(void **state) {
  BOARD * b = initial_board();

  assert_int_equal(8902, perft(b, 0, 3, 0));
}

static void perft_unit_test4(void **state) {
  BOARD * b = initial_board();

  assert_int_equal(197281, perft(b, 0, 4, 0));
}

static void perft_unit_test5(void **state) {
  BOARD * b = initial_board();

  assert_int_equal(4865609, perft(b, 0, 5, 0));
}

static void perft_unit_test6(void **state) {
  BOARD * b = initial_board();

  assert_int_equal(119060324, perft(b, 0, 6, 0));
}

static void perft_unit_test_talkchess5(void **state) {
  BOARD * b = parse_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");

  assert_int_equal(89941194, perft(b, 0, 5, 0));
}

static void forcing_moves_count_test(void **state)
{
  BOARD * b = parse_fen("2r3k1/1P2P3/8/8/4p3/3B4/8/6K1 w - - 0 1");
  int count = 0;

  while (moves(b, 0, NULL, NULL, MOVEGEN_FORCING_ONLY, count == 0)) {
    count ++;
  }

  assert_int_equal(8, count);
}

static void forcing_moves_test(void **state) {
  BOARD * b = parse_fen("2r3k1/1P2P3/8/8/4p3/3B4/8/6K1 w - - 0 1");
  MOVE * ptr;
  const char * mvs[] = {
    "d3e4", "e7e8q", "e7e8r", "b7c8n", "b7c8b", "b7c8r", "b7c8q", "d3c4"
  };
  int first = 1;

  while ((ptr = moves(b, 0, NULL, NULL, MOVEGEN_FORCING_ONLY, first))) {
    char buffer[6];
    int match = 0;

    first = 0;
    print_move_buffer(ptr, buffer);

    for (int i = 0; i < 8; ++i) {
      if (0 == strcmp(mvs[i], buffer)) {
        match = 1;
        break;
      }
    }

    assert_true(match);
  }
}

static void see_capture_test(void ** state) {
  BOARD * b = parse_fen("3Q4/3q4/1B2N3/5N2/2KPk3/3r4/2n1nb2/3R4 b - - 0 1");
  MOVE c2d4;

  c2d4.from            = (BITBOARD)1 << 10;
  c2d4.to              = (BITBOARD)1 << 27;
  c2d4.special         = ((BITBOARD)KNIGHT << PIECE_MOVE_SHIFT)
                       | ((BITBOARD)PAWN << CAPTURED_MOVE_SHIFT)
                       | b->en_passant;

  assert_int_equal(-200, see(b, &c2d4));
}

static void see_test(void ** state) {
  BOARD * b = parse_fen("7k/2b5/8/8/2N5/1R6/8/7K w - - 0 4");
  MOVE b3b6;

  b3b6.from            = (BITBOARD)1 << 17;
  b3b6.to              = (BITBOARD)1 << 41;
  b3b6.special         = ((BITBOARD)ROOK << PIECE_MOVE_SHIFT) | b->en_passant;

  assert_int_equal(-160, see(b, &b3b6));
}

int main(void) {
  int result;
  initialize_magic();
  initialize_in_between();
  initialize_hash();
  initialize_mat_tables();

  const struct CMUnitTest tests[] = {
    cmocka_unit_test(perft_unit_test1),
    cmocka_unit_test(perft_unit_test2),
    cmocka_unit_test(perft_unit_test3),
    cmocka_unit_test(perft_unit_test4),
    cmocka_unit_test(perft_unit_test5),
    cmocka_unit_test(perft_unit_test6),
    cmocka_unit_test(perft_unit_test_talkchess5),
    cmocka_unit_test(forcing_moves_count_test),
    cmocka_unit_test(forcing_moves_test),
    cmocka_unit_test(see_capture_test),
    cmocka_unit_test(see_test),

    cmocka_unit_test(pawns_test1),
    cmocka_unit_test(pawns_test2),
    cmocka_unit_test(pawns_test3),
    cmocka_unit_test(pawns_test4),
    cmocka_unit_test(pawns_test5),

    cmocka_unit_test(uci_parser_test1),
    cmocka_unit_test(uci_parser_test2),
    cmocka_unit_test(uci_parser_test3),
    cmocka_unit_test(uci_parser_test4),
    cmocka_unit_test(uci_parser_test5),
    cmocka_unit_test(uci_parser_test6),
    cmocka_unit_test(uci_parser_test7),
    cmocka_unit_test(uci_parser_test8),
    cmocka_unit_test(uci_parser_test9),
    cmocka_unit_test(uci_parser_test10),
    cmocka_unit_test(uci_parser_test11),
    cmocka_unit_test(uci_parser_test12),
    cmocka_unit_test(uci_parser_test13),
    cmocka_unit_test(uci_parser_test14),
    cmocka_unit_test(uci_parser_test15),
    cmocka_unit_test(uci_parser_test16),

    cmocka_unit_test(zobrist_test1),
    cmocka_unit_test(zobrist_test2),

    cmocka_unit_test(mat_tables_test1),
    cmocka_unit_test(mat_tables_test2),
    cmocka_unit_test(mat_tables_test3),
    cmocka_unit_test(mat_tables_test4),
    cmocka_unit_test(mat_tables_test5),
    cmocka_unit_test(mat_tables_test6),
    cmocka_unit_test(mat_tables_test7),
    cmocka_unit_test(mat_tables_test8),
    cmocka_unit_test(mat_tables_test9),
    cmocka_unit_test(mat_tables_test10),
    cmocka_unit_test(mat_tables_test11),
    cmocka_unit_test(mat_tables_test12),
    cmocka_unit_test(mat_tables_test13),
    cmocka_unit_test(mat_tables_test14),
    cmocka_unit_test(mat_tables_test15),
    cmocka_unit_test(mat_tables_test16),
    cmocka_unit_test(mat_tables_test17),
    cmocka_unit_test(mat_tables_test18),
    cmocka_unit_test(mat_tables_test19),
    cmocka_unit_test(mat_tables_test20),

    cmocka_unit_test(evaluate_test1),
    cmocka_unit_test(evaluate_test2),
    cmocka_unit_test(evaluate_test3),
    cmocka_unit_test(evaluate_test4),
    cmocka_unit_test(evaluate_test5),

    cmocka_unit_test(attacks_test1),
    cmocka_unit_test(attacks_test2),
    cmocka_unit_test(attacks_test3),
    cmocka_unit_test(attacks_test4),
    cmocka_unit_test(attacks_test5),
    cmocka_unit_test(attacks_test6),
    cmocka_unit_test(attacks_test7),
    cmocka_unit_test(attacks_test8),
    cmocka_unit_test(attacks_test9),
    cmocka_unit_test(attacks_test10),
    cmocka_unit_test(attacks_test11),
    cmocka_unit_test(attacks_test12),
    cmocka_unit_test(attacks_test13),
    cmocka_unit_test(attacks_test14),
    cmocka_unit_test(attacks_test15),
    cmocka_unit_test(attacks_test16),
    cmocka_unit_test(attacks_test17),
    cmocka_unit_test(attacks_test18),
    cmocka_unit_test(attacks_test19),
    cmocka_unit_test(attacks_test20),
  };

  result = cmocka_run_group_tests(tests, NULL, NULL);

  free(mat_table);

  return result;
}
