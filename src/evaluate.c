#include <strings.h>
#include <stdio.h>

#include "evaluate.h"
#include "pawns.h"
#include "board.h"
#include "mat_tables.h"

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

/* bishop and knight check mate on light squares (added to opponents king)*/
/* for light squares look up mirrored */
static const int king_bn_mate_dsq[] = {
  -90,-90,-70,-70,-30,-30, 10, 20,
  -90,-70,-70,-30,  0,  0, 10, 10,
  -70,-70,-10,  0,  0,  0,  0,-30,
  -70,-30,  0,  0,  0,  0,  0,-30,
  -30,  0,  0,  0,  0,  0,-30,-70,
  -30,  0,  0,  0,  0,-10,-70,-70,
   10, 10,  0,  0,-30,-70,-70,-90,
   20, 10,-30,-30,-70,-70,-90,-90
};

static const int* bonuses[] = {
  NULL,
  NULL,
  knight_bonus,
  bishop_bonus,
  rook_bonus,
  queen_bonus,
};

int psqt_value(PIECE piece, COLOUR colour, SQUARE from, SQUARE to) {
  if (piece < KNIGHT || piece > QUEEN) {
    return 0;
  }

  if (colour == WHITE) {
    from = ((63 - from) & ~0x7) | (from & 0x7);
    to   = ((63 - to) & ~0x7) | (to & 0x7);
  }

  return bonuses[piece][to] - bonuses[piece][from];
}

static inline int king_evaluate(const MAT_TABLE_ENTRY * mt, COLOUR colour, SQUARE kings[6], const BOARD * board);

int evaluate(const BOARD * board) {
  int value = 0;
  int dir[] = {1, -1};
  const MAT_TABLE_ENTRY * mt = get_mat_table_entry(board);
  SQUARE kings[6];

  if (mt->flags & DRAWN) {
    return 0;
  }

  DEBUG_PRINT("mat table score\t\t%d\n", mt->value);
  DEBUG_PRINT("mat table flags\t\t%8.8x\n", mt->flags);

  value += mt->value;

  for (COLOUR colour = WHITE; colour <= BLACK; colour++) {
    int pawn_value = 0;
    static const int rank_values[] = {0, 30, 35, 45, 65, 105, 185, 270};

    BITBOARD my_pawns    = board->pawns & COLOUR_BB(board, colour);
    BITBOARD their_pawns = board->pawns & COLOUR_BB(board, 1 - colour);

    BITBOARD iso  = isolated(my_pawns);
    BITBOARD pass = passers(my_pawns, their_pawns, colour);
    BITBOARD wk   = weak(my_pawns, colour);

    DEBUG_PRINT("%s isolated\t\t%8.8lx\n", colour_names[colour], iso);
    DEBUG_PRINT("%s passer\t\t%8.8lx\n", colour_names[colour], pass);
    DEBUG_PRINT("%s weak\t\t%8.8lx\n", colour_names[colour], wk);

    pawn_value += piece_values[PAWN] * __builtin_popcountll(my_pawns);
    pawn_value -= 20 * __builtin_popcountll(iso);
    while (pass) {
      BITBOARD isolated = pass & -pass;
      SQUARE sq         = __builtin_ctzll(isolated);
      SQUARE rank       = sq >> 3;
      SQUARE erank      = colour == WHITE ? rank : 7 - rank;

      pawn_value += rank_values[erank];

      pass &= pass - 1;
    }
    pawn_value -= 10 * __builtin_popcountll(wk);

    DEBUG_PRINT("%s pawn\t\t%d\n", colour_names[colour], pawn_value);

    value += dir[colour] * pawn_value;
  }

  for (COLOUR colour = WHITE; colour <= BLACK; colour++) {
    for (PIECE piece = PAWN + 1; piece < KING; ++piece) {
      BITBOARD pieces = *(&board->pawns + piece - PAWN) & COLOUR_BB(board, colour);

      while (pieces) {
        BITBOARD single = pieces & -pieces;
        SQUARE sq   = __builtin_ctzll(single);
        SQUARE rank = sq >> 3;
        SQUARE file = sq & 7;

        if (colour == WHITE) {
          sq = ((7 - rank) << 3) | file;
          value += piece_values[piece] + bonuses[piece][sq];
        }
        else {
          value -= piece_values[piece] + bonuses[piece][sq];
        }
        DEBUG_PRINT("%s %s\t\t%d\n",
            colour_names[colour], piece_names[piece], piece_values[piece] + bonuses[piece][sq]);

        pieces &= pieces - 1;
      }
    }
  }

  for (COLOUR colour = WHITE; colour <= BLACK; ++colour) {
    BITBOARD king = board->kings & COLOUR_BB(board, colour);
    SQUARE sq = __builtin_ctzll(king);
    SQUARE rank = sq >> 3;
    SQUARE file = sq & 7;

    kings[3 * colour] = sq;
    kings[3 * colour + 1] = file;
    kings[3 * colour + 2] = rank;
  }

  value += king_evaluate(mt, WHITE, kings, board) - king_evaluate(mt, BLACK, kings, board);

  DEBUG_PRINT("total\t\t\t%d\n", value);

  return (board->next == WHITE ? value : -value);
}

/* kings: square, file, rank, square, file, rank */
static inline int king_evaluate(const MAT_TABLE_ENTRY * mt, COLOUR colour, SQUARE kings[6], const BOARD * board) {
  int score;
  int endgame_factor = 0;

  /* simple piece checkmate */
  if (mt->flags & (W_CHECKMATING << (colour))) {
    /* Chebyshev distance of kings */
    int dist = MAX(ABS(kings[1] - kings[4]), ABS(kings[2] - kings[5]));
    score = 80 - 10 * dist;

    DEBUG_PRINT("%s king distance\t\t%d\n", colour_names[colour], score);

    return score;
  }

  if (mt->flags & (W_CHECKMATING << (colour ^ 1) )) {
    /* bishop / knight mate in light square corner */
    if (mt->flags & BN_MATE_LSQ) {
      score = king_bn_mate_dsq[((7 - kings[3 * colour + 2]) << 3) + kings[3 * colour + 1]];

      DEBUG_PRINT("%s king getting B+N checkmated %d\n", colour_names[colour], score);
      return score;
    }
    /* bishop / knight mate in dark square corner */
    else if (mt->flags & BN_MATE_DSQ) {
      int score = king_bn_mate_dsq[kings[3 * colour]];

      DEBUG_PRINT("%s king getting B+N checkmated %d\n", colour_names[colour], score);
      return score;
    }
  }

  SQUARE sqs[] = { ((7 - kings[2]) << 3) + kings[1], kings[3] };
  SQUARE sq = sqs[colour];

  /* default - piece square interpolate between middle game - endgame */
  endgame_factor = mt->flags & ENDGAME_MASK;
  static const int shield_value[] = { -40, -20, 0, 0 };
  int safety = __builtin_popcountll(
      shield((board->pawns | board->bishops) & COLOUR_BB(board, colour), /* fianchettoed bishops */
             colour,
             board->kings & COLOUR_BB(board, colour))
      ) * shield_value[endgame_factor];
  DEBUG_PRINT("%s king shield\t%d\n", colour_names[colour], safety);

  static const int mdl_psqt_weight[] = { 2, 2, 1, 0};
  static const int end_psqt_weight[] = { 0, 0, 1, 2};

  score = (mdl_psqt_weight[endgame_factor] * king_middlegame_bonus[sq] +
           end_psqt_weight[endgame_factor] * king_endgame_bonus[sq]) / 2;
  DEBUG_PRINT("%s king psqt\t\t%d\n", colour_names[colour], score);

  return score + safety;
}

