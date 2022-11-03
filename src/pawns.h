#ifndef __PAWNS_H__
#define __PAWNS_H__

#include "chess.h"
/* pawn structure evaluation */

/* isolated pawns */
BITBOARD isolated(BITBOARD pawns);

/* unopposed passers */
BITBOARD passers(BITBOARD our_pawns, BITBOARD their_pawns, COLOUR colour);

/* weak pawns - not defended by other pawns */
BITBOARD weak(BITBOARD pawns, COLOUR colour);

#endif /* ifndef __PAWNS_H__ */
