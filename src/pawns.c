#include "pawns.h"
#include "chess.h"
#include "attacks.h"

#define AFILE 0x0101010101010101ULL
#define BFILE 0x0202020202020202ULL
#define CFILE 0x0404040404040404ULL
#define DFILE 0x0808080808080808ULL
#define EFILE 0x1010101010101010ULL
#define FFILE 0x2020202020202020ULL
#define GFILE 0x4040404040404040ULL
#define HFILE 0x8080808080808080ULL

static BITBOARD fill(BITBOARD board) {
  board |= board << 8;
  board |= board << 16;
  board |= board << 32;
  board |= board >> 8;
  board |= board >> 16;
  board |= board >> 32;

  return board;
}

BITBOARD isolated(BITBOARD pawns) {
  BITBOARD filled = fill(pawns);

  return pawns & ~(((filled & ~ AFILE) >> 1) | ((filled & ~ HFILE) << 1));
}

static BITBOARD frontfill(BITBOARD board, COLOUR colour) {
  switch (colour) {
    case WHITE:
      board |= board << 8;
      board |= board << 16;
      board |= board << 32;

      return board;
    case BLACK:
      board |= board >> 8;
      board |= board >> 16;
      board |= board >> 32;
  }

  return board;
}

static BITBOARD frontspan(BITBOARD board, COLOUR colour) {
  board = frontfill(board, colour);
  return ((colour == WHITE) ? board << 8 : board >> 8);
}

static BITBOARD rearfill(BITBOARD board, COLOUR colour) {
  return frontfill(board, 1 - colour);
}

static BITBOARD rearspan(BITBOARD board, COLOUR colour) {
  board = rearfill(board, colour);
  return ((colour == WHITE) ? board >> 8 : board << 8);
}

BITBOARD passers(BITBOARD our_pawns, BITBOARD their_pawns, COLOUR colour) {
  /* frontline pawns - most advanced our pawns in a given file */
  BITBOARD frontline = ~rearspan(our_pawns, colour) & our_pawns;
  /* squares enemy pawns are capable of covering (ignoring pawns switching files by capturing) */
  BITBOARD enemycover = frontspan(their_pawns, 1 - colour);

  enemycover |= ((enemycover & ~AFILE) >> 1) | ((enemycover & ~HFILE << 1));

  return frontline & ~ enemycover;
}

BITBOARD weak(BITBOARD pawns, COLOUR colour) {
  BITBOARD captures = pawn_captures(pawns, colour);
  return pawns & ~ captures;
}

BITBOARD shield(BITBOARD our_pawns, COLOUR colour, BITBOARD king) {
  BITBOARD shield = 0;
  BITBOARD captures = pawn_captures(our_pawns, colour);

  switch (colour) {

    case WHITE:
      shield = (king << 16) | ((king & ~ AFILE) << 15) | ((king & ~(AFILE | BFILE)) << 14) |
                               ((king & ~HFILE) << 17) | ((king & ~(HFILE | GFILE)) << 18);
      break;

    case BLACK:
      shield = (king >> 16) | ((king & ~ HFILE) >> 15) | ((king & ~(HFILE | GFILE)) >> 14) |
                               ((king & ~AFILE) >> 17) | ((king & ~(AFILE | BFILE)) >> 18);
      break;
  }

  return shield & ~ captures;

}
