#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include <stdint.h>

#include "board.h"
#include "chess.h"
#include "move.h"
#include "killer.h"
#include "pv.h"

typedef enum {
  MOVEGEN_START = 0,
  MOVEGEN_PV,
  MOVEGEN_KILLER1,
  MOVEGEN_KILLER2,
  MOVEGEN_FORCING,
  MOVEGEN_FORCING_YIELD,
  MOVEGEN_OTHER,
  MOVEGEN_OTHER_YIELD,
} MOVEGEN_PHASE;

/* TODO this is temporary while the rest of the moves are in 1 phase */
typedef enum {
  MOVEGEN_PERFT,
  MOVEGEN_SORT,
  MOVEGEN_QUIESCE
} MOVEGEN_TYPE;

#define MOVEGEN_FLAGS_FRAME_OPEN     1
#define MOVEGEN_FLAGS_PAWN_PUSH      2
#define MOVEGEN_FLAGS_PAWN_FORCING   4
#define MOVEGEN_FLAGS_CASTLE         8

#define MOVEGEN_IX_64BIT_LANES       2

typedef struct __MOVEGEN_STATE__ {
  MOVEGEN_PHASE phase;
  MOVEGEN_TYPE movegen_type; /* TODO this is temporary while the rest of the moves are in 1 phase */
  unsigned flags;

  /* per piece type bits indexed by to square of moves, the move with such
   * target sq / piece type has already been generated
   */
  BITBOARD generated[7];

  /* single per move flags indexed by move index from movelist stored in
   * consecutive 64 bit numbers. Yielded is set when the move has been given to
   * the caller, not_forcing is set when the move has been generated, but
   * hasn't been yielded by the forcing phase.
   */
  uint64_t yielded[MOVEGEN_IX_64BIT_LANES];
  uint64_t not_forcing[MOVEGEN_IX_64BIT_LANES];
} MOVEGEN_STATE;

#if DEBUG
extern unsigned long long phase_counts[8];
#endif

MOVE * moves(const BOARD * board, int ply, const PV * pv, const KILLER * killer, MOVEGEN_STATE * state);
void moves_done(const MOVEGEN_STATE * state);

/* TODO move these */
CASTLE castle_update(const BOARD * board, PIECE piece, BITBOARD fromto);
BITBOARD castling_rook_from_to(CASTLE castle);

#endif /* ifndef _MOVEGEN_H_ */
