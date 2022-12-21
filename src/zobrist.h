#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include <stdint.h>

#include "chess.h"

typedef uint64_t HASH;

extern HASH rand_hash_colour_flip;

void initialize_hash();

HASH hash_colour(COLOUR colour);
HASH hash_castle(CASTLE castle);
HASH hash_en_passant(BITBOARD bitboard);
HASH hash_piece(BITBOARD bitboard, PIECE piece, COLOUR colour);

#endif /* ifndef __ZOBRIST_H__ */
