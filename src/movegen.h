#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include "board.h"
#include "chess.h"
#include "killer.h"
#include "pv.h"

typedef enum {
  MOVEGEN_START,
  MOVEGEN_PV,
  MOVEGEN_KILLER1,
  MOVEGEN_KILLER2,
  MOVEGEN_REST,
  MOVEGEN_REST_YIELD,
} MOVEGEN_PHASE;

/* TODO this is temporary while the rest of the moves are in 1 phase */
typedef enum {
  MOVEGEN_PERFT,
  MOVEGEN_SORT,
  MOVEGEN_FORCING
} MOVEGEN_TYPE;


typedef struct __MOVEGEN_STATE__ {
  MOVEGEN_PHASE phase;
  MOVEGEN_TYPE movegen_type; /* TODO this is temporary while the rest of the moves are in 1 phase */
  MOVE * iterator;
  int frame_open;
  BITBOARD generated[7];
} MOVEGEN_STATE;

MOVE * moves(const BOARD * board, int ply, const PV * pv, const KILLER * killer, MOVEGEN_STATE * state);
void moves_done(const MOVEGEN_STATE * state);


/* TODO move these */
CASTLE castle_update(const BOARD * board, PIECE piece, BITBOARD fromto);
BITBOARD castling_rook_from_to(CASTLE castle);

#endif /* ifndef _MOVEGEN_H_ */
