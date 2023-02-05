#include "movegen.h"

#include <strings.h>

#include "chess.h"
#include "see.h"
#include "evaluate.h"
#include "move.h"
#include "board.h"
#include "movelist.h"
#include "attacks.h"

static BITBOARD normal_attacks(const BOARD * board, PIECE piece, SQUARE sq, BITBOARD allowed_targets) {
  BITBOARD occ = OCCUPANCY_BB(board);

  switch (piece) {

    case KNIGHT:
      return knight_attacks[sq] & allowed_targets;

    case BISHOP:
      return bishop_bitboard(sq, occ) & allowed_targets;

    case ROOK:
      return rook_bitboard(sq, occ) & allowed_targets;

    case QUEEN:
      return (bishop_bitboard(sq, occ) | rook_bitboard(sq, occ)) & allowed_targets;

    case KING:
      return king_attacks[sq] & allowed_targets;

    default:
      return 0;
  }
}

CASTLE castle_update(const BOARD * board, PIECE piece, BITBOARD fromto) {
  CASTLE castle =
    ((fromto & ((BITBOARD)1 << 0)) << 1)   | ((fromto & ((BITBOARD)1 << 7)) >> 7) |
    ((fromto & ((BITBOARD)1 << 56)) >> 53) | ((fromto & ((BITBOARD)1 << 63)) >> 61);

  if (piece == KING) {
    castle |= (SHORT_CASTLE | LONG_CASTLE) << (board->next * 2);
  }

  return board->castle ^ (board->castle & ~castle);
}

/* knight, bishop, rook, queen and king moves excluding specials like castling */
static void add_normal_moves(const BOARD * board, PIECE piece, BITBOARD allowed_targets) {
  BITBOARD colour  = NEXT_COLOUR_BB(board);
  BITBOARD pieces  = *(& board->pawns + (piece - PAWN)) & colour;

  while (pieces) {
    BITBOARD from = pieces & - pieces;
    SQUARE   f    = ffsl(from) - 1;

    BITBOARD attacks  = normal_attacks(board, piece, f, allowed_targets);
    BITBOARD attacked = attacks & ~ colour;

    while (attacked) {
      BITBOARD to = attacked & - attacked;

      MOVE * move = ml_allocate();

      move->from            = from;
      move->to              = to;
      move->special         = ((BITBOARD)piece << PIECE_MOVE_SHIFT)
                            | board->en_passant
                            | (((BITBOARD)castle_update(board, piece, from | to) << CASTLE_RIGHT_CHANGE_SHIFT))
                            | (((BITBOARD)piece_at_board(board, to)) << CAPTURED_MOVE_SHIFT);

      attacked &= attacked - 1;
    }

    pieces &= pieces - 1;
  }
}

#define PROMOTIONS ((BITBOARD)0xff000000000000ff)

static void add_pawn_captures(const BOARD * board) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD targets;
  BITBOARD opp    = COLOUR_BB(board, 1 - board->next);
  BITBOARD pawns  = board->pawns & colour;

  /* captures - no promotion */
  targets = pawn_captures(pawns, board->next) & opp & ~PROMOTIONS;

  while (targets) {
    BITBOARD to = targets & - targets;
    BITBOARD f  = pawn_captures(to, 1 - board->next) & pawns;

    while (f) {
      BITBOARD from = f & -f;

      MOVE * move = ml_allocate();

      move->from            = from;
      move->to              = to;
      move->special         = ((BITBOARD)PAWN << PIECE_MOVE_SHIFT)
                            | board->en_passant
                            | (((BITBOARD)piece_at_board(board, to)) << CAPTURED_MOVE_SHIFT);

      f &= f - 1;
    }

    targets &= targets - 1;
  }

  /* captures - promotions */
  targets = pawn_captures(pawns, board->next) & opp & PROMOTIONS;

  while (targets) {
    BITBOARD to = targets & - targets;
    BITBOARD f  = pawn_captures(to, 1 - board->next) & pawns;

    while (f) {
      BITBOARD from = f & -f;

      for (PIECE piece = QUEEN; piece > PAWN; --piece) {
        MOVE * move = ml_allocate();

        move->from            = from;
        move->to              = to;

        move->special         = ((BITBOARD)PAWN << PIECE_MOVE_SHIFT)
                              | board->en_passant
                              | ((BITBOARD)piece << PROMOTION_MOVE_SHIFT)
                              | (((BITBOARD)piece_at_board(board, to)) << CAPTURED_MOVE_SHIFT)
                              | (((BITBOARD)castle_update(board, piece, from | to) << CASTLE_RIGHT_CHANGE_SHIFT));
      }

      f &= f - 1;
    }

    targets &= targets - 1;
  }

  /* en passant captures */
  targets = board->en_passant;
  if (targets) {
    BITBOARD f = pawn_captures(targets, 1 - board->next) & pawns;
    BITBOARD s = SINGLE_PAWN_PUSH(1 - board->next, targets);

    while (f) {
      BITBOARD from = f & -f;

      MOVE * move = ml_allocate();

      move->from            = from;
      move->to              = targets;
      move->special         = (((BITBOARD)PAWN) << PIECE_MOVE_SHIFT)
                            | board->en_passant
                            | s;

      f &= f - 1;
    }
  }
}


void add_pawn_promotions(const BOARD * board) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD pawns  = board->pawns & colour;
  BITBOARD empty  = ~ OCCUPANCY_BB(board);
  BITBOARD s      = single_pawn_pushes(pawns, empty, board->next);
  BITBOARD targets;

  /* single pawn pushes - promotions */
  targets = s & PROMOTIONS;
  while(targets) {
    BITBOARD to = targets & - targets;
    BITBOARD from = board->next == WHITE ? to >> 8 : to << 8;

    for (PIECE piece = QUEEN; piece > PAWN; --piece) {
      MOVE * move = ml_allocate();

      move->from            = from;
      move->to              = to;
      move->special         = ((BITBOARD)PAWN << PIECE_MOVE_SHIFT)
                            | ((BITBOARD)piece << PROMOTION_MOVE_SHIFT)
                            | board->en_passant;
    }

    targets &= targets - 1;
  }
}

void add_pawn_pushes(const BOARD * board) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD pawns  = board->pawns & colour;
  BITBOARD empty  = ~ OCCUPANCY_BB(board);
  BITBOARD s      = single_pawn_pushes(pawns, empty, board->next);
  BITBOARD targets;

  /* single pawn pushes - no promotions */
  targets = s & ~PROMOTIONS;
  while (targets) {

    MOVE * move = ml_allocate();

    BITBOARD to = targets & -targets;

    move->from            = SINGLE_PAWN_PUSH(1 - board->next, to);
    move->to              = to;
    move->special         = ((BITBOARD)PAWN << PIECE_MOVE_SHIFT) | board->en_passant;

    targets &= targets - 1;
  }

  /* double pawn pushes */
  targets = double_pawn_pushes(pawns, empty, board->next);
  while (targets) {
    MOVE * move = ml_allocate();

    BITBOARD to         = targets & - targets;
    BITBOARD en_passant = SINGLE_PAWN_PUSH(1 - board->next, to);
    BITBOARD from       = SINGLE_PAWN_PUSH(1 - board->next, en_passant);

    move->from            = from;
    move->to              = to;
    move->special         = ((BITBOARD)PAWN << PIECE_MOVE_SHIFT) | (board->en_passant ^ en_passant);

    targets &= targets - 1;
  }
}

static const BITBOARD castle_king_from_to[4] = {
  0x0000000000000050, 0x0000000000000014, 0x5000000000000000, 0x1400000000000000
};

static const BITBOARD castle_rook_from_to[4] = {
  0x00000000000000a0, 0x0000000000000009, 0xa000000000000000, 0x0900000000000000
};

BITBOARD castling_rook_from_to(CASTLE castle) {
  return castle_rook_from_to[castle];
}

/* from 0000...010.....010.... you get
 *      0000000001111111000000
 * (assuming two bits are set)
 */
#define DROPLO(bb)   ((bb) & ((bb) - 1))
#define ISOLATE(bb)  ((bb) & -(bb))
#define BITBOARD_BETWEEN(bb) ((DROPLO(bb) - 1) & ~(ISOLATE(bb) | (ISOLATE(bb) - 1)))

void add_castles(const BOARD * board) {
  BITBOARD occ = OCCUPANCY_BB(board);

  for (CASTLE castle = 0; castle <= 1; ++castle) {
    CASTLE c = (board->next << 1) | castle;

    if (board->castle & ((CASTLE)1 << c)) {
      BITBOARD rb = BITBOARD_BETWEEN(castle_rook_from_to[c]);
      BITBOARD kb = BITBOARD_BETWEEN(castle_king_from_to[c]);

      if (! ((rb | kb) & occ)) {

        BITBOARD check_squares = kb | castle_king_from_to[c];

        if (is_attacked(board, check_squares, occ, 1 - board->next)) continue;

        MOVE * move = ml_allocate();

        move->from    = castle_king_from_to[c] & board->kings;
        move->to      = castle_king_from_to[c] & ~board->kings;
        move->special = ((BITBOARD)KING << PIECE_MOVE_SHIFT)
                      | castle_rook_from_to[c]
                      | board->en_passant
                      | (((BITBOARD)castle_update(board, KING, castle_king_from_to[c]) << CASTLE_RIGHT_CHANGE_SHIFT));
      }
    }
  }
}

#if DEBUG
unsigned long long phase_counts[8] = { 0 };
#endif

#define IS_SET_IN_64(store, ix) ((store[(ix) >> 6]) & (1ULL << ((ix) & 63)))
#define SET_IN_64(store, ix) ((store[(ix) >> 6]) |= (1ULL << ((ix) & 63)))

void generate_move(const BOARD * board, const MOVE * move, MOVEGEN_STATE * state) {

  PIECE piece = (move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT;
  BITBOARD to = move->to;

  switch (piece) {
    case PAWN: {
      if (move->special & (CAPTURED_MOVE_MASK | EN_PASSANT_CAPTURE_MOVE_MASK | PROMOTION_MOVE_MASK)) {
        if (! (state->flags & MOVEGEN_FLAGS_PAWN_FORCING)) {
          state->flags |= MOVEGEN_FLAGS_PAWN_FORCING;
          add_pawn_captures(board);
          add_pawn_promotions(board);
        }
      } else {
        if (! (state->flags & MOVEGEN_FLAGS_PAWN_PUSH)) {
          state->flags |= MOVEGEN_FLAGS_PAWN_PUSH;
          add_pawn_pushes(board);
        }
      }

      break;
    }

    default: {
      if (move->special & CASTLE_ROOK_MOVE_MASK) {
        if (! (state->flags & MOVEGEN_FLAGS_CASTLE)) {
          state->flags |= MOVEGEN_FLAGS_CASTLE;
          add_castles(board);
        }
      } else if (! (state->generated[piece] & to)) {
        state->generated[piece] |= to;
        add_normal_moves(board, piece, to);
      }
    }
  }
}

static MOVE * yield_move(const MOVE * move, MOVEGEN_STATE * state) {
  MOVE * first = ml_first();
  int count = ml_last() - first;

  for (int lane = 0; lane < MOVEGEN_IX_64BIT_LANES; ++lane) {
    int count_in_lane = count >> (lane * 6);
    uint64_t to_yield = (~state->yielded[lane] & ((1ULL << count_in_lane) - 1));

    while (to_yield) {
      uint64_t iso = to_yield & -to_yield;
      int ix = __builtin_ctzll(iso) + lane * 64;

      MOVE * candidate = &first[ix];

      if (MOVE_EQUAL(move, candidate)) {

        state->yielded[lane] |= iso;

        return candidate;
      }

      to_yield &= to_yield - 1;
    }
  }

  return NULL;
}

static MOVE * yield_max_weight(uint64_t enabled[MOVEGEN_IX_64BIT_LANES], MOVEGEN_STATE * state) {
  MOVE * first     = ml_first();
  int count        = ml_last() - first;
  MOVE * max_move  = NULL;
  MOVEVAL max      = 0;
  int max_lane     = 0;
  uint64_t max_bit = 0;

  for (int lane = 0; lane < MOVEGEN_IX_64BIT_LANES; ++lane) {
    int count_in_lane = count >> (lane * 6);
    uint64_t to_yield = (~enabled[lane] & ((1ULL << count_in_lane) - 1));

    while (to_yield) {
      uint64_t iso = to_yield & -to_yield;
      int ix = __builtin_ctzll(iso) + lane * 64;

      MOVE * candidate = &first[ix];

      if (candidate->value > max) {
        max_move = candidate;
        max      = candidate->value;
        max_lane = lane;
        max_bit  = iso;
      }

      to_yield &= to_yield - 1;
    }
  }

  if (max_move) {
    state->yielded[max_lane] |= max_bit;
  }

  return max_move;
}

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
 * FSM
 * +-------+
 * | START +-------+    Perft and quiesce can skip PV and Killer moves. Negascout can
 * +-+-----+       |    generate PV and Killer first. If there is no killer store we
 *   |             |    skip to forcing moves.
 * +-v--+          |    We generate requested moves by piece type and target square.
 * | PV +--------+ |    When a move is generated the target squares are accumulated in
 * +-+--+        | |    the state. We avoid generating moves for set bits in subsequent
 *   |           | |    calls.
 * +-v-------+   | |    For forcing and other we have to generate all remaining moves
 * | KILLER1 <---+-+    for the phase because otherwise we couldn't guarantee right move
 * +-+-------+   | |    order.
 *   |           | |    Moves already yielded are marked by MOVE_VALUE_YIELDED flag.
 * +-v-------+   | |
 * | KILLER2 |   | |
 * +-+-------+   | |
 *   |           | |
 * +-v-------+   | |
 * | FORCING <---v-+
 * +-+-------+
 *   |
 * +-v-----+
 * | OTHER |
 * +-------+
 */
MOVE * moves(const BOARD * board, int ply, const PV * pv, const KILLER * killer, MOVEGEN_STATE * state) {
  MOVEGEN_PHASE next = state->phase;

  while (1) {
    switch (next) {
      case MOVEGEN_START:
        ml_open_frame();

        state->flags = MOVEGEN_FLAGS_FRAME_OPEN;

        for (PIECE piece = KNIGHT; piece <= KING; ++piece) {
          state->generated[piece] = 0ULL;
        }

        for (int i = 0; i < MOVEGEN_IX_64BIT_LANES; ++i) {
          state->yielded[i]     = 0ULL;
          state->not_forcing[i] = 0ULL;
        }

        if (pv) {
          next = MOVEGEN_PV;
        } else if (killer) {
          next = MOVEGEN_KILLER1;
        } else {
          next = MOVEGEN_FORCING;
        }
        break;

      case MOVEGEN_PV: {
        const MOVE * pv_move = pv_get_move(pv, ply);

        if (killer) {
          next = MOVEGEN_KILLER1;
        } else {
          next = MOVEGEN_FORCING;
        }

        if (pv_move) {
          MOVE * yielded;

          generate_move(board, pv_move, state);

          if ((yielded = yield_move(pv_move, state))) {
            yielded->value = 20000;
            state->phase   = next;

            return yielded;
          }
        }
        break;
      }

      case MOVEGEN_KILLER1: {
        const MOVE * killer_move = killer_get_move(killer, ply, 0);
        next = MOVEGEN_KILLER2;

        if (killer_move) {
          MOVE * yielded;

          generate_move(board, killer_move, state);

          if ((yielded = yield_move(killer_move, state))) {
            yielded->value = 19000;
            state->phase   = next;

            return yielded;
          }
        }
      }
      /* FALLTHROUGH */

      case MOVEGEN_KILLER2: {
        const MOVE * killer_move = killer_get_move(killer, ply, 1);
        next = MOVEGEN_FORCING;

        if (killer_move) {
          MOVE * yielded;

          generate_move(board, killer_move, state);

          if ((yielded = yield_move(killer_move, state))) {
            yielded->value = 18000;
            state->phase   = next;

            return yielded;
          }
        }
      }
      /* FALLTHROUGH */

      case MOVEGEN_FORCING: {
        SQUARE king = __builtin_ctzll(board->kings & COLOUR_BB(board, board->next ^ 1));
        int ix = 0;

        /* TODO add checks */
        if (! (state->flags & MOVEGEN_FLAGS_PAWN_FORCING)) {
          add_pawn_captures(board);
          add_pawn_promotions(board);
        }
        for (PIECE piece = KNIGHT; piece <= KING; ++piece) {
          /* captures */
          BITBOARD generate = COLOUR_BB(board, board->next ^ 1) & ~state->generated[piece];
          add_normal_moves(board, piece, generate);
          state->generated[piece] |= generate;
        }

        for (MOVE * move = ml_first(); move != ml_last(); move++) {
          if (!IS_SET_IN_64(state->yielded, ix)) {
            if (move->special & CAPTURED_MOVE_MASK) {
              move->value = see(board, move) + 10000;
            } else if (move->special & PROMOTION_MOVE_MASK) {
              PIECE promo = (move->special & PROMOTION_MOVE_MASK) >> PROMOTION_MOVE_SHIFT;
              move->value = piece_values[promo] + 9000;
            } else if (move->special & EN_PASSANT_CAPTURE_MOVE_MASK) {
              move->value = 8000;
            } else if (move_attacks_sq(board, move, king)) {
              move->value = 10000;
            } else {
              SET_IN_64(state->not_forcing, ix);
            }
          }
          ix++;
        }

        next = MOVEGEN_FORCING_YIELD;
        /* FALLTHROUGH */
      }

      case MOVEGEN_FORCING_YIELD: {
        MOVE * result = NULL;
        uint64_t flags[MOVEGEN_IX_64BIT_LANES];

        for (int i = 0; i < MOVEGEN_IX_64BIT_LANES; ++i) {
          flags[i] = state->yielded[i] | state->not_forcing[i];
        }

        if ((result = yield_max_weight(flags, state))) {
          state->phase = next;
          return result;
        } else {
          if (state->movegen_type == MOVEGEN_QUIESCE) {
            ml_close_frame();
            return NULL;
          } else {
            next = MOVEGEN_OTHER;
          }
        }

        /* FALLTHROUGH */
      }

      case MOVEGEN_OTHER: {
        SQUARE king = __builtin_ctzll(board->kings & COLOUR_BB(board, board->next ^ 1));
        int ix = 0;

        if (! (state->flags & MOVEGEN_FLAGS_PAWN_PUSH)) {
          add_pawn_pushes(board);
        }
        if (! (state->flags & MOVEGEN_FLAGS_CASTLE)) {
          add_castles(board);
        }
        for (PIECE piece = KNIGHT; piece <= KING; ++piece) {
          /* non-captures */
          BITBOARD generate = ~(COLOUR_BB(board, board->next) | state->generated[piece]);
          add_normal_moves(board, piece, generate);
        }

        /* assign psqt heuristics to moves */
        for (MOVE * move = ml_first(); move != ml_last(); move++) {

          if (!IS_SET_IN_64(state->yielded, ix)) {
            assert(! (move->special & CAPTURED_MOVE_MASK));
            /* TODO for instance a castling that attacks the king or a pawn push is only found here */
            /* and thus missed from quiesce now */
            if (move_attacks_sq(board, move, king)) {
              move->value = 1000;
            } else {
              move->value = psqt_value((move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT,
                  board->next,
                  (SQUARE)__builtin_ctzll(move->from),
                  (SQUARE)__builtin_ctzll(move->to)) + 500;
            }
          }
          ix++;
        }

        next = MOVEGEN_OTHER_YIELD;
        state->phase = next;
        /* FALLTHROUGH */
      }

      case MOVEGEN_OTHER_YIELD: {
        MOVE * result = NULL;

        if (!(result = yield_max_weight(state->yielded, state))) {
          ml_close_frame();
        }
        return result;
      }

      default:;
    }
  }
}

void moves_done(const MOVEGEN_STATE * mg_state) {
#if DEBUG
  phase_counts[mg_state->phase]++;
#endif
  if (mg_state->flags & MOVEGEN_FLAGS_FRAME_OPEN) {
    ml_close_frame();
  }
}

