#include <stdlib.h>

#include "evaluate.h"
#include "pawns.h"

static const int piece_values[] = { 0, 100, 320, 330, 500, 900 };

static const int pawn_bonus[] = {
   0,  0,  0,  0,  0,  0,  0,  0,
  50, 50, 50, 50, 50, 50, 50, 50,
  10, 10, 20, 30, 30, 20, 10, 10,
   5,  5, 10, 25, 25, 10,  5,  5,
   0,  0,  0, 20, 20,  0,  0,  0,
   5, -5,-10,  0,  0,-10, -5,  5,
   5, 10, 10,-20,-20, 10, 10,  5,
   0,  0,  0,  0,  0,  0,  0,  0
};

static const int knight_bonus[] = {
  -50,-40,-30,-30,-30,-30,-40,-50,
  -40,-20,  0,  0,  0,  0,-20,-40,
  -30,  0, 10, 15, 15, 10,  0,-30,
  -30,  5, 15, 20, 20, 15,  5,-30,
  -30,  0, 15, 20, 20, 15,  0,-30,
  -30,  5, 10, 15, 15, 10,  5,-30,
  -40,-20,  0,  5,  5,  0,-20,-40,
  -50,-40,-30,-30,-30,-30,-40,-50,
};

static const int bishop_bonus[] = {
  -20,-10,-10,-10,-10,-10,-10,-20,
  -10,  0,  0,  0,  0,  0,  0,-10,
  -10,  0,  5, 10, 10,  5,  0,-10,
  -10,  5,  5, 10, 10,  5,  5,-10,
  -10,  0, 10, 10, 10, 10,  0,-10,
  -10, 10, 10, 10, 10, 10, 10,-10,
  -10,  5,  0,  0,  0,  0,  5,-10,
  -20,-10,-10,-10,-10,-10,-10,-20,
};

static const int rook_bonus[] = {
  0,  0,  0,  0,  0,  0,  0,  0,
  5, 10, 10, 10, 10, 10, 10,  5,
  -5,  0,  0,  0,  0,  0,  0, -5,
  -5,  0,  0,  0,  0,  0,  0, -5,
  -5,  0,  0,  0,  0,  0,  0, -5,
  -5,  0,  0,  0,  0,  0,  0, -5,
  -5,  0,  0,  0,  0,  0,  0, -5,
  0,  0,  0,  5,  5,  0,  0,  0
};

static const int queen_bonus[] = {
  -20,-10,-10, -5, -5,-10,-10,-20,
  -10,  0,  0,  0,  0,  0,  0,-10,
  -10,  0,  5,  5,  5,  5,  0,-10,
  -5,  0,  5,  5,  5,  5,  0, -5,
  0,  0,  5,  5,  5,  5,  0, -5,
  -10,  5,  5,  5,  5,  5,  0,-10,
  -10,  0,  5,  0,  0,  0,  0,-10,
  -20,-10,-10, -5, -5,-10,-10,-20
};

static const int king_middlegame_bonus[] = {
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -20,-30,-30,-40,-40,-30,-30,-20,
  -10,-20,-20,-20,-20,-20,-20,-10,
  20, 20,  0,  0,  0,  0, 20, 20,
  20, 30, 10,  0,  0, 10, 30, 20
};

static const int king_endgame_bonus[] = {
  -50,-40,-30,-20,-20,-30,-40,-50,
  -30,-20,-10,  0,  0,-10,-20,-30,
  -30,-10, 20, 30, 30, 20,-10,-30,
  -30,-10, 30, 40, 40, 30,-10,-30,
  -30,-10, 30, 40, 40, 30,-10,-30,
  -30,-10, 20, 30, 30, 20,-10,-30,
  -30,-30,  0,  0,  0,  0,-30,-30,
  -50,-30,-30,-30,-30,-30,-30,-50
};

static const int* bonuses[] = {
  NULL,
  pawn_bonus,
  knight_bonus,
  bishop_bonus,
  rook_bonus,
  queen_bonus,
  king_middlegame_bonus,
  king_endgame_bonus
};

int evaluate(const BOARD * board) {
  int value = 0;
  int dir[] = {1, -1};

  for (COLOUR colour = WHITE; colour <= BLACK; colour++) {
    for (PIECE piece = PAWN; piece <= KING; ++piece) {
      BITBOARD pieces = *(&board->pawns + piece - PAWN) & COLOUR_BB(board, colour);

      while (pieces) {
        BITBOARD single = pieces & -pieces;
        SQUARE sq   = ffsl(single) - 1;

        if (colour == WHITE) {
          SQUARE rank = sq >> 3;
          SQUARE file = sq & 7;

          sq = ((7 - rank) << 3) | file;
          value += piece_values[piece] + bonuses[piece][sq];
        }
        else {
          value -= piece_values[piece] + bonuses[piece][sq];
        }

        pieces &= pieces - 1;
      }
    }

    /* isolated pawns */
    {
      int count = isolated_count(board->pawns & COLOUR_BB(board, colour));

      value -= dir[colour] * 33 * count;
    }

    /* passers */
    {
      BITBOARD my_pawns = board->pawns & COLOUR_BB(board, colour);
      BITBOARD their_pawns = board->pawns & COLOUR_BB(board, 1 - colour);
      BITBOARD bb = passers(my_pawns, their_pawns, colour);
      static const int rank_values[] = {0, 30, 35, 45, 65, 105, 185, 345};

      while (bb != 0) {
        BITBOARD single = bb & - bb;
        SQUARE rank = (ffsl(single) - 1) >> 3;

        if (colour == BLACK) {
          rank = 7 - rank;
        }

        value += dir[colour] * rank_values[rank];

        bb &= bb - 1;
      }

    }
  }

  return (board->next == WHITE ? value : -value);
}

