#include <assert.h>

#include "chess.h"
#include "see.h"
#include "attacks.h"

/* make bishops worth more because the evaluation gives bonuses to bishop pairs,
 * so we have a better chance of getting the order right
 * thus beta cutting more
 **/
static const int piece_values_mod[] = {NO_PIECE_V, PAWN_V, KNIGHT_V, BISHOP_V + 40, ROOK_V, QUEEN_V, KING_V};

int see(const BOARD * board, const MOVE * move) {
  BITBOARD from = move->from;
  BITBOARD to   = move->to;
  SQUARE tosq   = __builtin_ctzll(to);
  int ply       = 0;

  BITBOARD attackers[2][7] = {
    /* WHITE */
    {
      0, /* NO_PIECE */
      pawn_captures(to, BLACK) & board->pawns & COLOUR_BB(board, WHITE),
      knight_attacks[tosq] & board->knights & COLOUR_BB(board, WHITE),
      0ULL, /* bishops */
      0ULL, /* rooks */
      0ULL, /* queens */
      king_attacks[tosq] & board->kings & COLOUR_BB(board, WHITE),
    },
    /* BLACK */
    {
      0, /* NO_PIECE */
      pawn_captures(to, WHITE) & board->pawns & COLOUR_BB(board, BLACK),
      knight_attacks[tosq] & board->knights & COLOUR_BB(board, BLACK),
      0ULL, /* bishops */
      0ULL, /* rooks */
      0ULL, /* queens */
      king_attacks[tosq] & board->kings & COLOUR_BB(board, BLACK),
    }
  };

  PIECE captures[MAX_PLIES];

  PIECE start[2] = { PAWN, PAWN };
  PIECE piece    = (move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT;
  BITBOARD occ   = COLOUR_BB(board, WHITE) | COLOUR_BB(board, BLACK);
  COLOUR side    = board->next;

  captures[ply++] = (move->special & CAPTURED_MOVE_MASK) >> CAPTURED_MOVE_SHIFT;
  captures[ply++] = piece;

  /* dummy mkmove */
  attackers[side][piece] &= ~from;
  occ &= ~from;
  side = 1 - side;

  while (1) {
    /* get least valueable attacker */
    switch (start[side]) {
      case PAWN:
        if (0UL != (from = attackers[side][PAWN])) {
          piece = PAWN;
          from &= -from;
          break;
        }

      case KNIGHT:
        start[side] = KNIGHT; /* no more pawns */

        if (0UL != (from = attackers[side][KNIGHT])) {
          piece = KNIGHT;
          from &= -from;
          break;
        }

      case BISHOP:
        start[side] = BISHOP; /* no more knights (always restart at least at bishops for occ change) */

        if (0UL == attackers[side][BISHOP]) {
          attackers[side][BISHOP] = bishop_bitboard(tosq, occ) & occ & board->bishops & COLOUR_BB(board, side);
        }
        if (0UL != (from = attackers[side][BISHOP])) {
          piece = BISHOP;
          from &= -from;
          break;
        }

      case ROOK:
        if (0UL == attackers[side][ROOK]) {
          attackers[side][ROOK] = rook_bitboard(tosq, occ)  & occ & board->rooks & COLOUR_BB(board, side);
        }
        if (0UL != (from = attackers[side][ROOK])) {
          piece = ROOK;
          from &= -from;
          break;
        }

      case QUEEN:
        if (0UL == attackers[side][QUEEN]) {
          attackers[side][QUEEN] = (bishop_bitboard(tosq, occ) | rook_bitboard(tosq, occ)) &
            occ & board->queens & COLOUR_BB(board, side);
        }
        if (0UL != (from = attackers[side][QUEEN])) {
          piece = QUEEN;
          from &= -from;
          break;
        }

      case KING:
        if (0UL != (from = attackers[side][KING])) {
          piece = KING;
          from &= -from;
          break;
        }

      default: {
        int value = 0;

        ply--;
        for (ply--; ply >= 0; ply--) {
          value = MAX(value, 0);
          value = piece_values_mod[captures[ply]] - value;
        }

        /* THE END */
        return value;
      }
    }

    assert(from);

    /* dummy mkmove*/
    captures[ply++] = piece;
    attackers[side][piece] &= ~from;
    occ &= ~from;
    side ^= 1;
  }

}
