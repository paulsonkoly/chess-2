#ifndef _MOVELIST_H_
#define _MOVELIST_H_

#include "board.h"
#include "move.h"
#include "killer.h"
#include "pv.h"

void ml_open_frame(void);

void ml_close_frame(void);

MOVE * ml_allocate(void);

MOVE * ml_sort(const BOARD * board, const PV * pv, const KILLER * killer, int depth);

MOVE * ml_first(void);

MOVE * ml_forcing(const BOARD * board);

#endif /* ifndef _MOVELIST_H_ */
