#ifndef _MOVE_H_
#define _MOVE_H_

#include <stdint.h>

#include "chess.h"

#define IS_CASTLE  ((CASTLE)1 << 4)

typedef struct _MOVE_ {
  SQUARE from;
  SQUARE to;
  PIECE  piece;
  PIECE  promotion;
  /* 4 byte */
  SQUARE next_en_passant;
  CASTLE castle;
  uint8_t pad[2];
  /* 4 byte */
  BITBOARD en_passant;
} MOVE;

void print_move(const MOVE * move);

#define MOVE_EQUAL(a, b)                                                            \
  ((((a)->castle & (b) -> castle & IS_CASTLE) && ((a)->castle == (b)->castle)) ||   \
   ((a)->to == (b)->to && (a)->from == (b)->from && (a)->piece == (b)->piece))

#define SINGLE_PAWN_PUSH(colour, bitboard) (((bitboard) << 8) >> ((colour) << 4))

#endif /* ifndef _MOVE_H_ */
