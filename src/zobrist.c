#include <stdlib.h>
#include <strings.h>

#include "zobrist.h"

static HASH rand_hash_colour[2];
HASH rand_hash_colour_flip;
static HASH rand_hash_castle[16];
static HASH rand_hash_en_passant[16];   /* a3-h3 a6-h6 */
static HASH rand_hash_pieces[2][7][64]; /* colour / piece type / square */

#define HASH_RAND_VAL                                                  \
  (((HASH)(rand() & 0xffff) << 48) | ((HASH)(rand() & 0xffff) << 32) | \
   ((HASH)(rand() & 0xffff) << 16) | ((HASH)(rand() & 0xffff)))

void initialize_hash(void) {
  int i, j, k;

  for (i = 0; i < 2;  ++i) rand_hash_colour[i] = HASH_RAND_VAL;
  rand_hash_colour_flip = rand_hash_colour[0] ^ rand_hash_colour[1];
  for (i = 0; i < 4; ++i) rand_hash_castle[i] = HASH_RAND_VAL;
  for (i = 0; i < 16; ++i) rand_hash_en_passant[i] = HASH_RAND_VAL;
  for (i = 0; i < 2;  ++i) {
    for (k = 0; k < 64; ++k) {
      rand_hash_pieces[i][NO_PIECE][k] = 0;
    }
    for (j = PAWN; j <= KING; ++j) {
      for (k = 0; k < 64; ++k) {
        rand_hash_pieces[i][j][k] = HASH_RAND_VAL;
      }
    }
  }
}

HASH hash_colour(COLOUR colour) {
  return rand_hash_colour[colour];
}

HASH hash_castle(CASTLE castle) {
  HASH result = 0;

  while (castle) {
    CASTLE isolated = castle & - castle;
    int idx = ffs(isolated) - 1;

    result ^= rand_hash_castle[idx];

    castle &= castle - 1;
  }

  return result;
}

HASH hash_en_passant(BITBOARD bitboard) {
  HASH result = 0;

  while (bitboard) {
    BITBOARD isolated = bitboard & - bitboard;
    SQUARE   square = ffsl(isolated) - 1;
    SQUARE   file = square & 0x7;
    SQUARE   rank = square >> 3;

    result ^= rand_hash_en_passant[(rank == 2 ? 0 : 8) | file];

    bitboard &= bitboard - 1;
  }

  return result;
}

HASH hash_piece(BITBOARD bitboard, PIECE piece, COLOUR colour) {
  HASH result = 0;

  while (bitboard) {
    BITBOARD isolated = bitboard & - bitboard;
    SQUARE   square = ffsl(isolated) - 1;

    result ^= rand_hash_pieces[colour][piece][square];

    bitboard &= bitboard - 1;
  }

  return result;
}
