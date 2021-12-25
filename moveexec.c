#include <stdio.h>
#include <stdlib.h>

#include "movegen.h"
#include "chess.h"
#include "board.h"
#include "move.h"

static const BITBOARD castle_king_flip[4] = {
  0x0000000000000050, 0x0000000000000014, 0x5000000000000000, 0x1400000000000000
};

static const BITBOARD castle_rook_flip[4] = {
  0x00000000000000a0, 0x0000000000000009, 0xa000000000000000, 0x0900000000000000,
};

static const BITBOARD promotion_mask[6] = {
  0x0000000000000000, 0x0000000000000000, 0xff000000000000ff,
  0xff000000000000ff, 0xff000000000000ff, 0xff000000000000ff
};

static CASTLE castle_update(BITBOARD squares) {
  return
    ((squares & ((BITBOARD)1 << 0)) << 1) | ((squares & ((BITBOARD)1 << 7)) >> 7) |
    ((squares & ((BITBOARD)1 << 56)) >> 53) | ((squares & ((BITBOARD)1 << 63)) >> 61);
}

void execute_move(BOARD * board, MOVE * move) {
  BITBOARD * my = (BITBOARD*)&board->by_colour + board->next;

  if (move->castle & IS_CASTLE) {
    CASTLE c = move->castle & ALL_CASTLES;

    BITBOARD king_flip = castle_king_flip[c];
    BITBOARD rook_flip = castle_rook_flip[c];

    board->kings ^= king_flip;
    board->rooks ^= rook_flip;
    *my ^= (king_flip | rook_flip);

    board->en_passant = NO_SQUARE;
    board->castle &= ~((SHORT_CASTLE | LONG_CASTLE) << (2 * board->next));

  } else {
    BITBOARD remove     = ~((BITBOARD) 1 << move->to);
    BITBOARD fromto     = ((BITBOARD)1 << move->from) | ((BITBOARD) 1 << move->to);
    BITBOARD * opp      = (BITBOARD*)&board->by_colour + (1 - board->next);
    BITBOARD * mypiece  = &board->pawns + (move->piece - 1);
    BITBOARD en_passant = ~SINGLE_PAWN_PUSH(1 - board->next, move->en_passant);
    BITBOARD promotion  = ((BITBOARD) 1 << move->to) & promotion_mask[move->promotion];

    board->pawns   &= remove & en_passant;
    board->knights &= remove;
    board->bishops &= remove;
    board->rooks   &= remove;
    board->queens  &= remove;
    *opp           &= remove & en_passant;

    *mypiece ^= fromto;
    *my      ^= fromto;

    board->pawns &= ~promotion;
    *(&(board->pawns)+move->promotion - 1) |= promotion;

    board->en_passant = move->next_en_passant;
    board->castle &= ~(move->castle | castle_update(fromto));
  }

  board->next = 1 - board->next;

#define DEBUG_MOVES
#ifdef DEBUG_MOVES
#define EXPECT_EQL(ba, bb)                                                           \
if ((ba) != (bb)) {                                                                  \
  printf("BOARD INCONSISTENCY - " #ba " != " #bb " %016lx != %016lx\n", (BITBOARD)ba, (BITBOARD)bb); \
  print_board(board);                                                                \
  print_move(move);                                                                  \
  exit(0);                                                                           \
}

  EXPECT_EQL(board->by_colour.whitepieces & board->by_colour.blackpieces, 0);
  EXPECT_EQL(OCCUPANCY_BB(board),
      board->pawns | board->knights | board->bishops | board->rooks | board->queens | board->kings);
  EXPECT_EQL(board->pawns   & (board->knights | board->bishops | board->rooks   | board->queens | board->kings), 0);
  EXPECT_EQL(board->knights & (board->pawns   | board->bishops | board->rooks   | board->queens | board->kings), 0);
  EXPECT_EQL(board->bishops & (board->pawns   | board->knights | board->rooks   | board->queens | board->kings), 0);
  EXPECT_EQL(board->rooks   & (board->pawns   | board->knights | board->bishops | board->queens | board->kings), 0);
  EXPECT_EQL(board->queens  & (board->pawns   | board->knights | board->bishops | board->rooks  | board->kings), 0);
  EXPECT_EQL(board->kings   & (board->pawns   | board->knights | board->bishops | board->rooks  | board->queens), 0);

#endif
}
