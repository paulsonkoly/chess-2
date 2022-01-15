#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include "board.h"
#include "move.h"
#include "chess.h"

#define ALL_MOVES 0
#define ONLY_CAPTURES 1

MOVE * add_least_valuable_attacker(const BOARD * board, const MOVE * capture);
void add_moves(const BOARD * board, int only_captures);

#endif /* ifndef _MOVEGEN_H_ */