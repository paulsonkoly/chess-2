#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "board.h"
#include "move.h"
#include "killer.h"

int negascout(const BOARD* board, int depth, int alpha, int beta, int colour, MOVE * pv, MOVE * npv, KILLER * killer);

int iterative_deepening(const BOARD * board, int max_depth);

#endif /* ifndef _SEARCH_H_ */
