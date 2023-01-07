#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "see.h"

#include "movelist.h"
#include "moveexec.h"
#include "movegen.h"

static const int piece_value[] = {0, 100, 325, 400, 500, 900, 10000};

static int see__(BOARD * board, const MOVE * move) {
  int value = 0;

  MOVE * response;

  if ((response = add_least_valuable_attacker(board, move))) {
    PIECE capture = (response->special & CAPTURED_MOVE_MASK) >> CAPTURED_MOVE_SHIFT;

    execute_move(board, response);

    value = piece_value[capture] - see__(board, response);

    undo_move(board, response);

    value = value < 0 ? 0 : value;
  }

  return value;
}

int see(BOARD * board, const MOVE * move) {
  PIECE capture = (move->special & CAPTURED_MOVE_MASK) >> CAPTURED_MOVE_SHIFT;

  int value = piece_value[capture];

  ml_open_frame();

  execute_move(board, move);

  value = piece_value[capture] - see__(board, move);

  undo_move(board, move);

  ml_close_frame();

  return value;
}

#include "attacks.h"

int see2(BOARD * board, const MOVE * move) {
  BITBOARD from = move->from;
  BITBOARD to   = move->to;
  SQUARE tosq   = __builtin_ctzll(to);

  BITBOARD attackers[2][6] = {
    /* WHITE */
    {
      0, /* NO_PIECE */
      pawn_captures(to, BLACK) & board->pawns & COLOUR_BB(board, WHITE),
      knight_attacks[tosq] & board->knights & COLOUR_BB(board, WHITE),
      0ULL, /* bishops */
      0ULL, /* rooks */
      0ULL  /* queens */
    },
    /* BLACK */
    {
      0, /* NO_PIECE */
      pawn_captures(to, WHITE) & board->pawns & COLOUR_BB(board, BLACK),
      knight_attacks[tosq] & board->knights & COLOUR_BB(board, BLACK),
      0ULL, /* bishops */
      0ULL, /* rooks */
      0ULL  /* queens */
    }
  };

  PIECE start[2] = { PAWN, PAWN };
  PIECE piece    = (move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT;
  BITBOARD occ   = COLOUR_BB(board, WHITE) | COLOUR_BB(board, BLACK);
  COLOUR side    = board->next;

  if (piece == KING) {
    return 0; /* we would be indexing outside of attackers */
  }

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
          from &= from;
          break;
        }

      case ROOK:
        if (0UL == attackers[side][ROOK]) {
          attackers[side][ROOK] = rook_bitboard(tosq, occ)  & occ & board->rooks & COLOUR_BB(board, side);
        }
        if (0UL != (from = attackers[side][ROOK])) {
          piece = ROOK;
          from &= from;
          break;
        }

      case QUEEN:
        if (0UL == attackers[side][QUEEN]) {
          attackers[side][QUEEN] = (bishop_bitboard(tosq, occ) | rook_bitboard(tosq, occ)) &
            occ & board->queens & COLOUR_BB(board, side);
        }
        if (0UL != (from = attackers[side][QUEEN])) {
          piece = QUEEN;
          from &= from;
          break;
        }

      default:
        /* THE END */
        return 0;
    }

    assert(from);

    {
      const char * x = " PNBRQ";
      SQUARE f = __builtin_ctzll(from);
      SQUARE rank = f / 8;
      SQUARE file = f & 7;

      printf("%c %c%d\n", x[piece], 'a' + file, rank + 1);
    }

    /* dummy mkmove*/
    attackers[side][piece] &= ~from;
    occ &= ~from;
    side ^= 1;
  }

}
