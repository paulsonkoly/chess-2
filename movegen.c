#include "movegen.h"

#include <strings.h>

#include "chess.h"
#include "move.h"
#include "board.h"
#include "movelist.h"
#include "attacks.h"

void add_knight_moves(const BOARD * board, BITBOARD allowed_targets) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD knights = board->knights & colour;

  BITBOARD_SCAN(knights) {
    SQUARE from = BITBOARD_SCAN_ITER(knights);
    BITBOARD attacks = knight_attacks[from] & allowed_targets;
    BITBOARD t = attacks & ~ colour;

    BITBOARD_SCAN(t) {
      MOVE * move = ml_allocate();

      SQUARE to = BITBOARD_SCAN_ITER(t);

      move->from            = from;
      move->to              = to;
      move->piece           = KNIGHT;
      move->promotion       = NO_PIECE;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;
      move->capture         = piece_at_board(board, to);
    }
  }
}

void add_bishop_moves(const BOARD * board, BITBOARD allowed_targets) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD bishops = board->bishops & colour;

  BITBOARD_SCAN(bishops) {
    SQUARE from = BITBOARD_SCAN_ITER(bishops);
    BITBOARD attacks = bishop_bitboard(board, from) & allowed_targets;
    BITBOARD t = attacks & ~ colour;

    BITBOARD_SCAN(t) {
      MOVE * move = ml_allocate();
      SQUARE to = BITBOARD_SCAN_ITER(t);

      move->from            = from;
      move->to              = to;
      move->piece           = BISHOP;
      move->promotion       = NO_PIECE;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;
      move->capture         = piece_at_board(board, to);
    }
  }
}

void add_rook_moves(const BOARD * board, BITBOARD allowed_targets) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD rooks = board->rooks & colour;

  BITBOARD_SCAN(rooks) {
    SQUARE from = BITBOARD_SCAN_ITER(rooks);
    BITBOARD attacks = rook_bitboard(board, from) & allowed_targets;
    BITBOARD t = attacks & ~ colour;

    BITBOARD_SCAN(t) {
      MOVE * move = ml_allocate();
      SQUARE   to = BITBOARD_SCAN_ITER(t);

      move->from            = from;
      move->to              = to;
      move->piece           = ROOK;
      move->promotion       = NO_PIECE;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;
      move->capture         = piece_at_board(board, to);
    }
  }
}

void add_queen_moves(const BOARD * board, BITBOARD allowed_targets) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD queens = board->queens & colour;

  BITBOARD_SCAN(queens) {
    SQUARE from = BITBOARD_SCAN_ITER(queens);
    BITBOARD attacks = (bishop_bitboard(board, from) | rook_bitboard(board, from)) & allowed_targets;
    BITBOARD t = attacks & ~ colour;

    BITBOARD_SCAN(t) {
      MOVE * move = ml_allocate();
      SQUARE to = BITBOARD_SCAN_ITER(t);

      move->from            = from;
      move->to              = to;
      move->piece           = QUEEN;
      move->promotion       = NO_PIECE;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;
      move->capture         = piece_at_board(board, to);
    }
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
  BITBOARD pawns  = board->pawns & colour;
  BITBOARD empty  = ~ OCCUPANCY_BB(board);
  BITBOARD s      = single_pawn_pushes(pawns, empty, board->next) & allowed_targets;
  BITBOARD t;

  t = s & ~PROMOTIONS;
  BITBOARD_SCAN(t) {
    MOVE * move = ml_allocate();
    SQUARE to = BITBOARD_SCAN_ITER(t);
    SQUARE from = to + 8 * (2 * board->next - 1);

    move->from            = from;
    move->to              = to;
    move->piece           = PAWN;
    move->promotion       = NO_PIECE;
    move->en_passant      = 0;
    move->next_en_passant = NO_SQUARE;
    move->castle          = NO_CASTLE;
    move->capture         = NO_PIECE;
  }

  t = s & PROMOTIONS;
  BITBOARD_SCAN(t) {
    SQUARE to = BITBOARD_SCAN_ITER(t);
    SQUARE from = to + 8 * (2 * board->next - 1);

    for (PIECE piece = QUEEN; piece > PAWN; --piece) {
      MOVE * move = ml_allocate();

      move->from            = from;
      move->to              = to;
      move->piece           = PAWN;
      move->promotion       = piece;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;
      move->capture         = NO_PIECE;
    }
  }

  t = double_pawn_pushes(pawns, empty, board->next) & allowed_targets;

  BITBOARD_SCAN(t) {
    MOVE * move = ml_allocate();
    SQUARE to = BITBOARD_SCAN_ITER(t);
    SQUARE from = to + 16 * (2 * board->next - 1);

    move->from            = from;
    move->to              = to;
    move->piece           = PAWN;
    move->promotion       = NO_PIECE;
    move->en_passant      = 0;
    move->next_en_passant = (from + to) / 2;
    move->castle          = NO_CASTLE;
    move->capture         = NO_PIECE;
  }

  {
    BITBOARD en_passant = board->en_passant == NO_SQUARE ? 0 : (BITBOARD)1 << board->en_passant;
    BITBOARD attacks = pawn_captures(pawns, board->next) ;
    BITBOARD opp = COLOUR_BB(board, 1 - board->next);
    BITBOARD t;

    attacks &= (opp | en_passant);

    t = attacks & ~PROMOTIONS;
    BITBOARD_SCAN(t) {
      SQUARE to = BITBOARD_SCAN_ITER(t);
      BITBOARD capture = (BITBOARD)1 << to;
      BITBOARD pawnbb = pawn_captures(capture, 1 - board->next) & pawns;

      BITBOARD_SCAN(pawnbb) {
        MOVE * move = ml_allocate();
        SQUARE from = BITBOARD_SCAN_ITER(pawnbb);

        move->from            = from;
        move->to              = to;
        move->piece           = PAWN;
        move->promotion       = NO_PIECE;
        move->en_passant      = en_passant & capture;
        move->next_en_passant = NO_SQUARE;
        move->castle          = NO_CASTLE;
        move->capture         = piece_at_board(board, to);
      }
    }

    t = attacks & PROMOTIONS;
    BITBOARD_SCAN(t) {
      SQUARE to = BITBOARD_SCAN_ITER(t);
      BITBOARD capture = (BITBOARD)1 << to;
      BITBOARD pawnbb;

      pawnbb = pawn_captures(capture, 1 - board->next) & pawns;

      BITBOARD_SCAN(pawnbb) {
        SQUARE from = BITBOARD_SCAN_ITER(pawnbb);

        for (PIECE piece = QUEEN; piece > PAWN; --piece) {
          MOVE * move = ml_allocate();

          move->from            = from;
          move->to              = to;
          move->piece           = PAWN;
          move->promotion       = piece;
          move->en_passant      = 0;
          move->next_en_passant = NO_SQUARE;
          move->castle          = NO_CASTLE;
          move->capture         = piece_at_board(board, to);

        }
      }
    }
  }
}

void add_king_moves(const BOARD * board, BITBOARD allowed_tergets) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD king = board->kings & colour;
  SQUARE from = ffsl(king) - 1;
  BITBOARD attacks = king_attacks[from] & allowed_tergets;
  BITBOARD t = attacks & ~ colour;

  BITBOARD_SCAN(t) {
    MOVE * move = ml_allocate();
    SQUARE to = BITBOARD_SCAN_ITER(t);

    move->from            = from;
    move->to              = to;
    move->piece           = KING;
    move->promotion       = NO_PIECE;
    move->en_passant      = 0;
    move->next_en_passant = NO_SQUARE;
    move->castle          = (SHORT_CASTLE | LONG_CASTLE) << (board->next * 2);
  }
}

static const BITBOARD castle_squares[4] = {
  0x0000000000000060, 0x000000000000000e, 0x6000000000000000, 0x0e00000000000000
};

static const BITBOARD castle_check_squares[4] = {
  0x0000000000000070, 0x000000000000001c, 0x7000000000000000, 0x1c00000000000000
};

void add_castles(const BOARD * board) {
  BITBOARD occ = OCCUPANCY_BB(board);

  for (CASTLE castle = 0; castle <= 1; ++castle) {
    CASTLE c = (board->next << 1) | castle;

    if (board->castle & ((CASTLE)1 << c)) {
      if (!(castle_squares[c] & occ)) {
        if (is_attacked(board, castle_check_squares[c], 1 - board->next)) continue;
        MOVE * move = ml_allocate();

        move->castle  = IS_CASTLE | c;
        move->capture = NO_PIECE;
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
    add_knight_moves(board, allowed_targets);
    add_king_moves(board, allowed_targets);
    add_bishop_moves(board, allowed_targets);
    add_rook_moves(board, allowed_targets);
    add_queen_moves(board, allowed_targets);
  }
}

