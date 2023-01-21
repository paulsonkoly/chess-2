#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#include "board.h"

#if DEBUG
int evaluate(const BOARD * board, int debug);
#define EVALUATE(board) evaluate(board, 0)
#else
int evaluate(const BOARD * board);
#define EVALUATE(board) evaluate(board)
#endif
int psqt_value(PIECE piece, COLOUR colour, SQUARE from, SQUARE to);

#endif /* ifndef _EVALUATE_H_ */
