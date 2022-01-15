#ifndef _MOVEEXEC_H_
#define _MOVEEXEC_H_

#include "board.h"
#include "move.h"

void execute_move(BOARD * board, const MOVE * move);
void undo_move(BOARD * board, const MOVE * move);

#endif /* ifndef _MOVEEXEC_H_ */
