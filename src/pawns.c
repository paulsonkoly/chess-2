#include "pawns.h"
#include "chess.h"

#define AFILE 0x0101010101010101ULL
#define BFILE 0x0202020202020202ULL
#define CFILE 0x0404040404040404ULL
#define DFILE 0x0808080808080808ULL
#define EFILE 0x1010101010101010ULL
#define FFILE 0x2020202020202020ULL
#define GFILE 0x4040404040404040ULL
#define HFILE 0x8080808080808080ULL

static BITBOARD neighbouring_files[] = {
  BFILE, AFILE | CFILE, BFILE | DFILE, CFILE | EFILE,
  DFILE | FFILE, EFILE | GFILE, FFILE| HFILE, GFILE,
};

int isolated_count(BITBOARD pawns) {
  int count = 0;
  BITBOARD remaining = pawns;

  while (remaining) {
    BITBOARD single = remaining & - remaining;
    SQUARE file = (ffsl(single) - 1) & 7;

    if ((neighbouring_files[file] & pawns) == 0) {
      count += 1;
    }

    remaining &= remaining - 1;
  }

  return count;
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
