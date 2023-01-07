#ifndef _ATTACKS_H_
#define _ATTACKS_H_

#include "chess.h"
#include "board.h"
#include "move.h"

extern const BITBOARD king_attacks[];
extern const BITBOARD knight_attacks[];

extern BITBOARD bishop_attacks[64][512];
extern BITBOARD rook_attacks[64][4096];

void initialize_magic();

BITBOARD bishop_bitboard(SQUARE sq, BITBOARD occ);
BITBOARD rook_bitboard(SQUARE sq, BITBOARD occ);

BITBOARD pawn_captures(BITBOARD pawns, COLOUR colour);

BITBOARD is_attacked(const BOARD * board, BITBOARD squares, COLOUR colour);
BITBOARD in_check(const BOARD * board, COLOUR colour);
/* does the move attack the square - either stepping out of an x-ray attack or directly */
int move_attacks_sq(const BOARD * board, const MOVE * move, SQUARE sq);

#endif /* ifndef _ATTACKS_H_ */
