#ifndef _MOVEEXEC_H_
#define _MOVEEXEC_H_

#include "board.h"
#include "move.h"

void execute_move(BOARD * board, MOVE * move);
void undo_move(BOARD * board, MOVE * move);

#endif /* ifndef _MOVEEXEC_H_ */
