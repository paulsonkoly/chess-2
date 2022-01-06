#ifndef _MOVELIST_H_
#define _MOVELIST_H_

#include "board.h"
#include "move.h"
#include "killer.h"

void ml_open_frame();

void ml_close_frame();

MOVE * ml_allocate();

MOVE * ml_sort(const MOVE * pv, int depth, const KILLER * killer);

MOVE * ml_first();

MOVE * ml_forcing(const BOARD * board);

#endif /* ifndef _MOVELIST_H_ */
