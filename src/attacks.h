#ifndef _ATTACKS_H_
#define _ATTACKS_H_

#include "chess.h"
#include "board.h"

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

#endif /* ifndef _ATTACKS_H_ */
