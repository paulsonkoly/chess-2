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

/* set on the rank 2 ranks up from the king NOT defended by our pawns */
/* king: g1, pawns: g2, h2. Set on e3 */
BITBOARD shield(BITBOARD our_pawns, COLOUR colour, BITBOARD king);

#endif /* ifndef __PAWNS_H__ */
