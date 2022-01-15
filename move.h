#ifndef _MOVE_H_
#define _MOVE_H_

#include <stdint.h>

#include "chess.h"

#define IS_CASTLE  ((CASTLE)1 << 4)

typedef uint8_t MOVEVAL;

typedef struct _MOVE_ {

  PIECE  piece;
  PIECE  promotion;

  PIECE  capture;
  CASTLE castle;

  BITBOARD from;
  BITBOARD to;
  BITBOARD special;
  BITBOARD en_passant;

  MOVEVAL value;

  struct _MOVE_ * next;
} MOVE;

void print_move(const MOVE * move);

#define MOVE_EQUAL(a, b)                                                            \
  ((a)->to == (b)->to && (a)->from == (b)->from && (a)->piece == (b)->piece)

#define SINGLE_PAWN_PUSH(colour, bitboard) (((bitboard) << 8) >> ((colour) << 4))

#endif /* ifndef _MOVE_H_ */
