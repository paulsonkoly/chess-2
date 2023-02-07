#ifndef _MOVELIST_H_
#define _MOVELIST_H_

#include "board.h"
#include "move.h"
#include "killer.h"

void ml_open_frame(void);

void ml_close_frame(void);

MOVE * ml_allocate(void);

MOVE * ml_sort(BOARD * board, const MOVE * pv, int depth, const KILLER * killer);

MOVE * ml_first(void);

MOVE * ml_forcing(BOARD * board);

#endif /* ifndef _MOVELIST_H_ */
