#ifndef _SEE_H_
#define _SEE_H_

#include "move.h"
#include "board.h"

int see(BOARD * board, const MOVE * move);
int see2(BOARD * board, const MOVE * move);
int see_capture(BOARD * board, const MOVE * move);

#endif /* ifndef _SEE_H_ */
