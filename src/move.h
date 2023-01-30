#ifndef _MOVE_H_
#define _MOVE_H_

#include "chess.h"

typedef int MOVEVAL;

typedef struct _MOVE_ {
  /* 0 - 7 [a1 - h1]   : white castle (high @ rook from/to)
   * 8-11  [a2 - d2]   : piece moving
   * 12-15 [e2 - h2]   : promotion piece
   * 16-23 [a3 - h3]   : en_passant change (high where ep changes, dropping or raising)
   * 24-31 [a4 - h4]   : en_passant capture
   * 32-39 [a5 - h5]   : en_passant capture
   * 40-47 [a6 - h6]   : en_passant change (high where ep changes, dropping or raising)
   * 48-51 [a7 - d7]   : captured piece - NO_PIECE for ep capture
   * 52-55 [e7 - h7]   : castle right change
   * 56-63 [a8 - h8]   : black castle (high @ rook from/to)
   */
#define CASTLE_ROOK_MOVE_MASK         0xff000000000000ffULL
#define PIECE_MOVE_MASK               0x0000000000000f00ULL
#define PIECE_MOVE_SHIFT              8
#define PROMOTION_MOVE_MASK           0x000000000000f000ULL
#define PROMOTION_MOVE_SHIFT          12
#define EN_PASSANT_CAPTURE_MOVE_MASK  0x000000ffff000000ULL
#define EN_PASSANT_CHANGE_MASK        0x0000ff0000ff0000ULL
#define CAPTURED_MOVE_MASK            0x000f000000000000ULL
#define CAPTURED_MOVE_SHIFT           48
#define CASTLE_RIGHT_CHANGE_MASK      0x00f0000000000000ULL
#define CASTLE_RIGHT_CHANGE_SHIFT     52
  BITBOARD special;

  BITBOARD from;
  BITBOARD to;
/* TODO remove all this */
#define MOVE_VALUE_MASK        0x0000ffff
#define MOVE_VALUE_YIELDED     0x10000000
#define MOVE_VALUE_NOT_FORCING 0x20000000
  MOVEVAL value;

  struct _MOVE_ * next;
} MOVE;

void print_move(const MOVE * move);
void print_move_buffer(const MOVE * move, char * buffer);

#define MOVE_EQUAL(a, b)                                                            \
  ((a)->to == (b)->to && (a)->from == (b)->from && (a)->special == (b)->special)

#define SINGLE_PAWN_PUSH(colour, bitboard) (((bitboard) << 8) >> ((colour) << 4))

#endif /* ifndef _MOVE_H_ */
