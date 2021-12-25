#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include "board.h"
#include "move.h"

#define ALL_MOVES 0
#define ONLY_CAPTURES 1

MOVE * add_moves(const BOARD * board, MOVE * moveptr, int only_captures);

BITBOARD in_check(const BOARD * board, COLOUR colour);

void initialize_magic();

#endif /* ifndef _MOVEGEN_H_ */
