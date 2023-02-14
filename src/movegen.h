#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include <stdint.h>

#include "board.h"
#include "chess.h"
#include "move.h"
#include "killer.h"
#include "pv.h"

/* what moves to generate */
typedef enum {
  MOVEGEN_NORMAL,        /* all moves */
  MOVEGEN_FORCING_ONLY   /* checks and captures only */
} MOVEGEN_TYPE;

/* Semi-lazy phase based move generator
 *
 * Yields all pseudo-legal moves to the caller one move at a time and once
 * there is no more moves it yields NULL.
 *
 * Generates pseudo-legal moves in a given position in phases. Phases are mainly
 * defined by the heuristic ordering of the moves where the order is roughly
 * defined as PV > Killer moves > forcing moves > other moves.
 * Forcing moves are defined as captures and checks.
 * The generator tries to
 *  (1) return as early as possible before generating less valued moves
 *  allowing to omit generation of such moves in case of a beta cut
 *  (2) avoid multiple generation of moves
 *  (3) avoid yielding the same move multiple times
 *  (4) return moves in heuristic order within a single phase, thus see (static
 *  exchange evaluation) heuristics and psqt (piece-square table) heuristics
 *  are maintained for forcing move / other move generation.
 *
 *  first should be set on the first call from a position then it shouldn't be
 *  set on subsequent calls
 */
MOVE * moves(const BOARD * board, int ply, const PV * pv, const KILLER * killer, MOVEGEN_TYPE type, int first);
/* call if a search returns early - before moves() returning NULL */
void moves_done(int ply);

/* TODO move these */
CASTLE castle_update(const BOARD * board, PIECE piece, BITBOARD fromto);
BITBOARD castling_rook_from_to(CASTLE castle);

#endif /* ifndef _MOVEGEN_H_ */
