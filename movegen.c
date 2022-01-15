#include "movegen.h"

#include <strings.h>

#include "chess.h"
#include "move.h"
#include "board.h"
#include "movelist.h"
#include "attacks.h"

static BITBOARD normal_attacks(const BOARD * board, PIECE piece, SQUARE sq, BITBOARD allowed_targets) {
  switch (piece) {

    case KNIGHT:
      return knight_attacks[sq] & allowed_targets;

    case BISHOP:
      return bishop_bitboard(board, sq) & allowed_targets;

    case ROOK:
      return rook_bitboard(board, sq) & allowed_targets;

    case QUEEN:
      return (bishop_bitboard(board, sq) | rook_bitboard(board, sq)) & allowed_targets;

    case KING:
      return king_attacks[sq] & allowed_targets;

    default:
      return 0;
  }
}

static CASTLE castle_update(const BOARD * board, PIECE piece, BITBOARD fromto) {
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
      move->special         = 0;

      move->piece           = piece;
      move->promotion       = NO_PIECE;
      move->en_passant      = board->en_passant;
      move->castle          = castle_update(board, piece, from | to);
      move->capture         = piece_at_board(board, to);

      attacked &= attacked - 1;
    }

    pieces &= pieces - 1;
  }
}

BITBOARD single_pawn_pushes(BITBOARD pawns, BITBOARD empty, COLOUR colour) {
  return SINGLE_PAWN_PUSH(colour, pawns) & empty;
}

static const BITBOARD double_pawn_push_ranks[2] = {
  0x00000000ff000000, 0x000000ff00000000
};

BITBOARD double_pawn_pushes(BITBOARD pawns, BITBOARD empty, COLOUR colour) {
  BITBOARD single_pushes = single_pawn_pushes(pawns, empty, colour);

  return SINGLE_PAWN_PUSH(colour,single_pushes) & empty & double_pawn_push_ranks[colour];
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
    move->special         = 0;

    move->piece           = PAWN;
    move->promotion       = NO_PIECE;
    move->en_passant      = board->en_passant;
    move->castle          = 0;
    move->capture         = NO_PIECE;

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
      move->special         = to;

      move->piece           = PAWN;
      move->promotion       = piece;
      move->en_passant      = board->en_passant;
      move->castle          = 0;
      move->capture         = NO_PIECE;
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
    move->special         = 0;

    move->piece           = PAWN;
    move->promotion       = NO_PIECE;
    move->en_passant      = board->en_passant ^ en_passant;
    move->castle          = 0;
    move->capture         = NO_PIECE;

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
      move->special         = s;

      move->piece           = PAWN;
      move->promotion       = NO_PIECE;
      move->en_passant      = board->en_passant;
      move->castle          = 0;
      move->capture         = PAWN;

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
      move->special         = 0;

      move->piece           = PAWN;
      move->promotion       = NO_PIECE;
      move->en_passant      = board->en_passant;
      move->castle          = 0;
      move->capture         = piece_at_board(board, to);

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
        move->special         = to;

        move->piece           = PAWN;
        move->promotion       = piece;
        move->en_passant      = board->en_passant;
        move->castle          = castle_update(board, PAWN, to);
        move->capture         = piece_at_board(board, to);
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

        if (is_attacked(board, check_squares, 1 - board->next)) continue;

        MOVE * move = ml_allocate();

        move->from       = castle_king_from_to[c] & board->kings;
        move->to         = castle_king_from_to[c] & ~board->kings;
        move->special    = castle_rook_from_to[c];

        move->piece      = KING;
        move->promotion  = NO_PIECE;
        move->en_passant = board->en_passant;
        move->castle     = IS_CASTLE | castle_update(board, KING, castle_king_from_to[c]);
        move->capture    = NO_PIECE;
      }
    }
  }
}

void add_moves(const BOARD * board, int only_captures) {
  BITBOARD allowed_targets = COLOUR_BB(board, 1 - board->next);

  switch (only_captures) {
  case 0:
    add_pawn_moves(board, 0xffffffffffffffff);
    add_castles(board);
    allowed_targets = ~NEXT_COLOUR_BB(board);
  case 1:
    for (PIECE piece = KNIGHT; piece <= KING; ++piece) {
      add_normal_moves(board, piece, allowed_targets);
    }
  }
}

