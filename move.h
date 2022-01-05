#ifndef _MOVE_H_
#define _MOVE_H_

#include <stdint.h>

#include "chess.h"

#define IS_CASTLE  ((CASTLE)1 << 4)

typedef uint8_t MOVEVAL;

typedef struct _MOVE_ {
  SQUARE from;
  SQUARE to;
  PIECE  piece;
  PIECE  promotion;
  /* 4 byte */
  PIECE  capture;
  SQUARE next_en_passant;
  CASTLE castle;
  MOVEVAL value;

  /* 4 byte */
  BITBOARD en_passant;

  /* 16 byte */
  struct _MOVE_ * next;
} MOVE;

void print_move(const MOVE * move);

#define MOVE_EQUAL(a, b)                                                            \
  ((((a)->castle & (b) -> castle & IS_CASTLE) && ((a)->castle == (b)->castle)) ||   \
   ((a)->to == (b)->to && (a)->from == (b)->from && (a)->piece == (b)->piece))

#define SINGLE_PAWN_PUSH(colour, bitboard) (((bitboard) << 8) >> ((colour) << 4))

#endif /* ifndef _MOVE_H_ */
