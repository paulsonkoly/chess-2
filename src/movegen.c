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
      move->value           = 0;

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
      move->value           = 0;

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
      move->value           = 0;

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
      move->value           = 0;
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
    move->value           = 0;

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
    move->value           = 0;

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

/* void add_moves(const BOARD * board, int only_captures) { */
/*   BITBOARD allowed_targets = COLOUR_BB(board, 1 - board->next); */

/*   switch (only_captures) { */
/*   case 0: */
/*     add_pawn_moves(board, 0xffffffffffffffff); */
/*     add_castles(board); */
/*     allowed_targets = ~NEXT_COLOUR_BB(board); */
/*   case 1: */
/*     for (PIECE piece = KNIGHT; piece <= KING; ++piece) { */
/*       add_normal_moves(board, piece, allowed_targets); */
/*     } */
/*   } */
/* } */

#if DEBUG
unsigned long long phase_counts[8] = { 0 };
#endif

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

        state->frame_open = 1;

        for (PIECE piece = PAWN; piece < KING; ++piece) {
          state->generated[piece] = 0;
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
          PIECE piece = (pv_move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT;
          BITBOARD to = pv_move->to;

          if (PAWN < piece && !(to & COLOUR_BB(board, board->next))) { /* TODO */
            state->generated[piece] |= to;
            add_normal_moves(board, piece, to);

            for (MOVE * move = ml_first(); move != ml_last(); move++) {
              if (MOVE_EQUAL(move, pv_move)) {
                move->value |= MOVE_VALUE_YIELDED;

                state->phase = next;
                return move;
              }
            }
          }
        }
        break;
      }

      case MOVEGEN_KILLER1: {
        const MOVE * killer_move = killer_get_move(killer, ply, 0);
        next = MOVEGEN_KILLER2;

        if (killer_move) {
          PIECE piece = (killer_move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT;
          BITBOARD to = killer_move->to;

          if (PAWN < piece && !(to & COLOUR_BB(board, board->next))) { /* TODO */
            BITBOARD generated = state->generated[piece];
            if (!(generated & to)) {
              add_normal_moves(board, piece, ~generated);
              state->generated[piece] = generated | to;
            }

            for (MOVE * move = ml_first(); move != ml_last(); move++) {
              if (!(move->value & MOVE_VALUE_YIELDED) && MOVE_EQUAL(move, killer_move)) {
                move->value |= MOVE_VALUE_YIELDED;

                state->phase = next;
                return move;
              }
            }
          }
        }
        /* FALLTHROUGH */
      }

      case MOVEGEN_KILLER2: {
        const MOVE * killer_move = killer_get_move(killer, ply, 1);
        next = MOVEGEN_FORCING;

        if (killer_move) {
          PIECE piece = (killer_move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT;
          BITBOARD to = killer_move->to;

          if (PAWN < piece && !(to & COLOUR_BB(board, board->next))) { /* TODO */
            BITBOARD generated = state->generated[piece];
            state->generated[piece] |= to;

            if (!(generated & to)) {
              add_normal_moves(board, piece, ~generated);
              state->generated[piece] = generated | to;
            }

            for (MOVE * move = ml_first(); move != ml_last(); move++) {
              if (!(move->value & MOVE_VALUE_YIELDED) && MOVE_EQUAL(move, killer_move)) {
                move->value |= MOVE_VALUE_YIELDED;

                state->phase = next;
                return move;
              }
            }
          }
        }
        /* FALLTHROUGH */
      }

      case MOVEGEN_FORCING: {
        SQUARE king = __builtin_ctzll(board->kings & COLOUR_BB(board, board->next ^ 1));

        add_pawn_captures(board);
        add_pawn_promotions(board);
        for (PIECE piece = KNIGHT; piece <= KING; ++piece) {
          /* captures */
          BITBOARD generate = COLOUR_BB(board, board->next ^ 1) & ~state->generated[piece];
          add_normal_moves(board, piece, generate);
          state->generated[piece] |= generate;
          /* TODO add checks */
        }

        for (MOVE * move = ml_first(); move != ml_last(); move++) {
          if (!(move->value & MOVE_VALUE_YIELDED)) {

            if (move->special & CAPTURED_MOVE_MASK) {
              move->value |= see(board, move) + 10000;
            } else if (move->special & PROMOTION_MOVE_MASK) {
              PIECE promo = (move->special & PROMOTION_MOVE_MASK) >> PROMOTION_MOVE_SHIFT;
              move->value |= piece_values[promo] + 9000;
            } else if (move->special & EN_PASSANT_CAPTURE_MOVE_MASK) {
              move->value |= 8000;
            } else if (move_attacks_sq(board, move, king)) {
              move->value |= 10000;
            } else {
              move->value |= MOVE_VALUE_NOT_FORCING;
            }
          }
        }

        next = MOVEGEN_FORCING_YIELD;
        /* FALLTHROUGH */
      }

      case MOVEGEN_FORCING_YIELD: {
        MOVE * result = NULL;
        MOVEVAL max = 0;

        for (MOVE * move = ml_first(); move != ml_last(); move++) {
          if (!(move->value & (MOVE_VALUE_YIELDED | MOVE_VALUE_NOT_FORCING))) {
            if (max < (move->value & MOVE_VALUE_MASK)) {
              max = move->value & MOVE_VALUE_MASK;
              result = move;
            }
          }
        }

        if (result) {
          result->value |= MOVE_VALUE_YIELDED;
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

        add_pawn_pushes(board);
        add_castles(board);
        for (PIECE piece = KNIGHT; piece <= KING; ++piece) {
          /* non-captures */
          BITBOARD generate = ~(COLOUR_BB(board, board->next) | state->generated[piece]);
          add_normal_moves(board, piece, generate);
        }

        /* assign psqt heuristics to moves */
        for (MOVE * move = ml_first(); move != ml_last(); move++) {

          if (!(move->value & MOVE_VALUE_YIELDED)) {
            assert(! (move->special & CAPTURED_MOVE_MASK));
            /* TODO for instance a castling that attacks the king or a pawn push is only found here */
            /* and thus missed from quiesce now */
            if (move_attacks_sq(board, move, king)) {
              move->value |= 1000;
            } else {
              move->value |= psqt_value((move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT,
                  board->next,
                  (SQUARE)__builtin_ctzll(move->from),
                  (SQUARE)__builtin_ctzll(move->to)) + 500;
            }
          }
        }

        next = MOVEGEN_OTHER_YIELD;
        /* FALLTHROUGH */
      }

      case MOVEGEN_OTHER_YIELD: {
        MOVE * result = NULL;
        MOVEVAL max = 0;

        for (MOVE * move = ml_first(); move != ml_last(); move++) {
          if (!(move->value & MOVE_VALUE_YIELDED)) {
            if (max < (move->value & MOVE_VALUE_MASK)) {
              max = move->value & MOVE_VALUE_MASK;
              result = move;
            }
          }
        }

        if (result) {
          result->value |= MOVE_VALUE_YIELDED;
          state->phase = next;
        } else {
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
  if (mg_state->frame_open) {
    ml_close_frame();
  }
}

