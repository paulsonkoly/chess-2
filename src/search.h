#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "board.h"
#include "move.h"
#include "killer.h"
#include "pv.h"

int negascout(BOARD* board,
    int ply,
    int depth,
    int reduced_depth,
    int alpha,
    int beta,
    const PV * opv,
    PV ** npv,
    KILLER * killer);

int iterative_deepening(BOARD * board, int max_depth);

#endif /* ifndef _SEARCH_H_ */
