#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#include "board.h"

int evaluate(const BOARD * board);
int psqt_value(PIECE piece, COLOUR colour, SQUARE from, SQUARE to);

#endif /* ifndef _EVALUATE_H_ */
