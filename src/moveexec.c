#include <stdio.h>
#include <stdlib.h>

#include "chess.h"
#include "board.h"
#include "move.h"

#ifndef NDEBUG
#define EXPECT_EQL(ba, bb)                                                           \
if ((ba) != (bb)) {                                                                  \
  printf("BOARD INCONSISTENCY - " #ba " != " #bb " %016lx != %016lx\n", (BITBOARD)ba, (BITBOARD)bb); \
  print_board(board);                                                                \
  print_move(move);                                                                  \
  abort();                                                                           \
}
#else
#define EXPECT_EQL(ba, bb)
#endif

static void flip_board_bits(BOARD * board, const MOVE * move);

void execute_move(BOARD * board, const MOVE * move) {
  flip_board_bits(board, move);

  board->next = 1 - board->next;
}

void undo_move(BOARD * board, const MOVE * move) {
  board->next = 1 - board->next;

  flip_board_bits(board, move);
}

static void flip_board_bits(BOARD * board, const MOVE * move) {
  PIECE piece     = (PIECE)((move->special & PIECE_MOVE_MASK) >> PIECE_MOVE_SHIFT);
  PIECE captured  = (PIECE)((move->special & CAPTURED_MOVE_MASK) >> CAPTURED_MOVE_SHIFT);
  PIECE promotion = (PIECE)((move->special & PROMOTION_MOVE_MASK) >> PROMOTION_MOVE_SHIFT);

  BITBOARD * my_piece   = & board->pawns + (piece - PAWN);
  BITBOARD * opp_colour = (BITBOARD *)(& board->by_colour) + (1 - board->next);
  BITBOARD * my_colour  = (BITBOARD *)(& board->by_colour) + board->next;

  BITBOARD fromto           = move->from | move->to;
  BITBOARD ep_capture       = move->special & EN_PASSANT_CAPTURE_MOVE_MASK;
  BITBOARD castle_rook_move = move->special & CASTLE_ROOK_MOVE_MASK;

  /* en-passant */
  board->pawns ^= ep_capture;
  *opp_colour  ^= ep_capture;

  if (captured) {
    BITBOARD * opp_piece  = & board->pawns + (captured - PAWN);

    /* remove captured piece */
    *opp_piece  ^= move->to;
    *opp_colour ^= move->to;
  }

  /* move self piece */
  *my_piece  ^= fromto;
  *my_colour ^= fromto;

  /* take down the pawn and put up a promotion piece */
  if (promotion) {
    BITBOARD * my_promo = & board->pawns + (promotion - PAWN);

    board->pawns ^= move->to;
    *my_promo    ^= move->to;
  }

  /* move castling rook */
  board->rooks  ^= castle_rook_move;
  *my_colour    ^= castle_rook_move;

  /* update castling rights */
  CASTLE castle = (CASTLE)((move->special & CASTLE_RIGHT_CHANGE_MASK) >> CASTLE_RIGHT_CHANGE_SHIFT);

  board->castle ^= castle;

  /* update en-passant state */
  board->en_passant ^= move->special & EN_PASSANT_CHANGE_MASK;

  EXPECT_EQL(board->by_colour.whitepieces & board->by_colour.blackpieces, 0);
  EXPECT_EQL(OCCUPANCY_BB(board),
      board->pawns | board->knights | board->bishops | board->rooks | board->queens | board->kings);
  EXPECT_EQL(board->pawns   & (board->knights | board->bishops | board->rooks   | board->queens | board->kings), 0);
  EXPECT_EQL(board->knights & (board->pawns   | board->bishops | board->rooks   | board->queens | board->kings), 0);
  EXPECT_EQL(board->bishops & (board->pawns   | board->knights | board->rooks   | board->queens | board->kings), 0);
  EXPECT_EQL(board->rooks   & (board->pawns   | board->knights | board->bishops | board->queens | board->kings), 0);
  EXPECT_EQL(board->queens  & (board->pawns   | board->knights | board->bishops | board->rooks  | board->kings), 0);
  EXPECT_EQL(board->kings   & (board->pawns   | board->knights | board->bishops | board->rooks  | board->queens), 0);
}
