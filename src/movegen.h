#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include "board.h"
#include "move.h"
#include "chess.h"

#define ALL_MOVES 0
#define ONLY_CAPTURES 1

void add_moves(const BOARD * board, int only_captures);

CASTLE castle_update(const BOARD * board, PIECE piece, BITBOARD fromto);
BITBOARD castling_rook_from_to(CASTLE castle);

#endif /* ifndef _MOVEGEN_H_ */
