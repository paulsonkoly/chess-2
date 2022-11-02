#ifndef __PAWNS_H__
#define __PAWNS_H__ value

#include "chess.h"
/* pawn structure evaluation */

/* number of isolated pawns */
int isolated_count(BITBOARD pawns);

/* unopposed passers */
BITBOARD passers(BITBOARD our_pawns, BITBOARD their_pawns, COLOUR colour);

#endif /* ifndef __PAWNS_H__ */
