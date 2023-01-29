#include "movegen.h"

#include <strings.h>

#include "chess.h"
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

/* set allowed targets to either all high or all low. all low for only captures */
void add_pawn_moves(const BOARD * board, BITBOARD allowed_targets) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD opp    = COLOUR_BB(board, 1 - board->next);
  BITBOARD pawns  = board->pawns & colour;
  BITBOARD empty  = ~ OCCUPANCY_BB(board);
  BITBOARD s      = single_pawn_pushes(pawns, empty, board->next) & allowed_targets;
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

  /* double pawn pushes */
  targets = double_pawn_pushes(pawns, empty, board->next) & allowed_targets;
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

  /* en passant captures */
  targets = board->en_passant & allowed_targets;
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

void add_moves(const BOARD * board) {
  BITBOARD allowed_targets = COLOUR_BB(board, 1 - board->next);

  add_pawn_moves(board, 0xffffffffffffffff);
  add_castles(board);
  allowed_targets = ~NEXT_COLOUR_BB(board);
  for (PIECE piece = KNIGHT; piece <= KING; ++piece) {
    add_normal_moves(board, piece, allowed_targets);
  }
}

#include <assert.h>

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
          next = MOVEGEN_REST;
        }
        break;

      case MOVEGEN_PV: {
        const MOVE * pv_move = pv_get_move(pv, ply);

        if (killer) {
          next = MOVEGEN_KILLER1;
        } else {
          next = MOVEGEN_REST;
        }

        if (pv_move) {
          PIECE piece = (pv_move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT;
          BITBOARD to = pv_move->to;

          if (PAWN < piece && !(to & COLOUR_BB(board, board->next))) { /* TODO */
            state->generated[piece] |= to;
            add_normal_moves(board, piece, to);

            for (MOVE * move = ml_first(); move != NULL; move = move->next) { /* TODO normal iteration */
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
            state->generated[piece] |= to;
            add_normal_moves(board, piece, to);

            for (MOVE * move = ml_first(); move != NULL; move = move->next) { /* TODO normal iteration */
              if (MOVE_EQUAL(move, killer_move)) {
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
        next = MOVEGEN_REST;

        if (killer_move) {
          PIECE piece = (killer_move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT;
          BITBOARD to = killer_move->to;

          if (PAWN < piece && !(to & COLOUR_BB(board, board->next))) { /* TODO */
            state->generated[piece] |= to;
            add_normal_moves(board, piece, to);

            for (MOVE * move = ml_first(); move != NULL; move = move->next) { /* TODO normal iteration */
              if (MOVE_EQUAL(move, killer_move)) {
                move->value |= MOVE_VALUE_YIELDED;

                state->phase = next;
                return move;
              }
            }
          }
        }
        /* FALLTHROUGH */
      }

      case MOVEGEN_REST: {
        BITBOARD no_friendly = ~ COLOUR_BB(board, board->next);
        /* TODO fix this nonsense */
        add_pawn_moves(board, 0xffffffffffffffff);
        add_castles(board);
        for (PIECE piece = KNIGHT; piece <= KING; ++piece) {
          add_normal_moves(board, piece, no_friendly & ~state->generated[piece]);
        }

        switch (state->movegen_type) {

          case MOVEGEN_PERFT:
            state->iterator = ml_first();
            break;

          case MOVEGEN_SORT: {
            state->iterator = ml_sort(board, pv, killer, ply);
            break;
          }

          case MOVEGEN_FORCING:
            state->iterator = ml_forcing(board);
            break;

          default:;
        }

        next = MOVEGEN_REST_YIELD;
        /* FALLTHROUGH */
      }

      case MOVEGEN_REST_YIELD: {
        MOVE * result = state->iterator;

        while (result && result->value & MOVE_VALUE_YIELDED) {
          result = result->next;
        }

        if (result) {
          state->iterator = result->next;
        } else {
          state->frame_open = 0;
          ml_close_frame();
        }

        state->phase = next;

        return result;
      }


      default:;
    }
  }
}

void moves_done(const MOVEGEN_STATE * mg_state) {
  if (mg_state->frame_open) {
    ml_close_frame();
  }
}

