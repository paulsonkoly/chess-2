#include <stdio.h>
#include <stdlib.h>

#include "chess.h"
#include "board.h"
#include "move.h"

#define DEBUG_MOVES
#ifdef DEBUG_MOVES
#define EXPECT_EQL(ba, bb)                                                           \
if ((ba) != (bb)) {                                                                  \
  printf("BOARD INCONSISTENCY - " #ba " != " #bb " %016lx != %016lx\n", (BITBOARD)ba, (BITBOARD)bb); \
  print_board(board);                                                                \
  print_move(move);                                                                  \
  exit(0);                                                                           \
}
#endif

void execute_move(BOARD * board, MOVE * move) {
  BITBOARD * opp_piece  = & board->pawns + (move->capture - PAWN);
  BITBOARD * opp_colour = (BITBOARD *)(& board->by_colour) + (1 - board->next);

  BITBOARD * my_piece   = & board->pawns + (move->piece - PAWN);
  BITBOARD * my_colour  = (BITBOARD *)(& board->by_colour) + board->next;

  BITBOARD fromto = move->from | move->to;

  /* en-passant */
  if (move->piece == PAWN && move->capture == PAWN && move->special) {
    /* remove opponent pawn */
    *opp_piece  ^= move->special;
    *opp_colour ^= move->special;

    /* move self piece */
    *my_piece  ^= fromto;
    *my_colour ^= fromto;
  } else {
    /* remove captured piece */
    if (move->capture) {
      *opp_piece  ^= move->to;
      *opp_colour ^= move->to;
    }

    /* move self piece */
    *my_piece  ^= fromto;
    *my_colour ^= fromto;

    /* move the rook in castling and update castling rights */
    if (move->castle & IS_CASTLE) {
      board->rooks  ^= move->special;
      *my_colour    ^= move->special;
    }

    board->castle ^= (move->castle & ALL_CASTLES);

    /* take down the pawn and put up a promotion piece */
    if (move->promotion) {
      BITBOARD * my_promo = & board->pawns + (move->promotion - PAWN);

      board->pawns ^= move->special;
      *my_promo    ^= move->special;
    }
  }

  /* update en-passant state */
  board->en_passant ^= move->en_passant;

  board->next = 1 - board->next;

#ifdef DEBUG_MOVES
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

void undo_move(BOARD * board, MOVE * move) {
  board->next = 1- board->next;

  BITBOARD * opp_piece  = & board->pawns + (move->capture - PAWN);
  BITBOARD * opp_colour = (BITBOARD *)(& board->by_colour) + (1 - board->next);

  BITBOARD * my_piece   = & board->pawns + (move->piece - PAWN);
  BITBOARD * my_colour  = (BITBOARD *)(& board->by_colour) + board->next;

  BITBOARD fromto = move->from | move->to;

  /* en-passant */
  if (move->piece == PAWN && move->capture == PAWN && move->special) {
    /* move self piece */
    *my_piece  ^= fromto;
    *my_colour ^= fromto;

    /* put opponent pawn back */
    *opp_piece  ^= move->special;
    *opp_colour ^= move->special;
  } else {
    /* take down the promotion piece and put up a pawn */
    if (move->promotion) {
      BITBOARD * my_promo = & board->pawns + (move->promotion - PAWN);

      board->pawns  ^= move->special;
      *my_promo     ^= move->special;
    }

    board->castle ^= (move->castle & ALL_CASTLES);

    /* move the rook in castling and update castling rights */
    if (move->castle & IS_CASTLE) {
      board->rooks  ^= move->special;
      *my_colour    ^= move->special;
    }

    /* move self piece back */
    *my_piece  ^= fromto;
    *my_colour ^= fromto;

    /* add back captured piece */
    if (move->capture) {
      *opp_piece  ^= move->to;
      *opp_colour ^= move->to;
    }
  }

  /* update en-passant state */
  board->en_passant ^= move->en_passant;

#ifdef DEBUG_MOVES
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
