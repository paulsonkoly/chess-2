#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>

#include <cmocka.h>

#include "pawns_tests.h"

#include "pawns.h"

void pawns_test1(void ** state) {
  BITBOARD pawns = 0x00000000488a8900ULL;
  /* isolated */
  assert_int_equal(0x0000000008080800ULL, isolated(pawns));
}

void pawns_test2(void ** state) {
  BITBOARD our_pawns = 0x00000000000a2a00ULL;
  BITBOARD their_pawns = 0x00c2000000000000ULL;
  /* passers */
  assert_int_equal(0x0000000000080000ULL, passers(our_pawns, their_pawns, WHITE));
}

void pawns_test3(void ** state) {
  BITBOARD pawns = 0x0000800020120900ULL;
  /* weak */
  assert_int_equal(0x0000800000000900ULL, weak(pawns, WHITE));
}

void pawns_test4(void ** state) {
  BITBOARD pawns = 0x000000100400d300ULL;
  BITBOARD king  = 0x0000000000000040ULL;
  /* shield */
  assert_int_equal(0x0000000000100000ULL, shield(pawns, WHITE, king));
}
