#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include "board.h"
#include "chess.h"

void add_moves(const BOARD * board);

CASTLE castle_update(const BOARD * board, PIECE piece, BITBOARD fromto);
BITBOARD castling_rook_from_to(CASTLE castle);

#endif /* ifndef _MOVEGEN_H_ */
