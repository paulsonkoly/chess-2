#ifndef _BOARD_H_
#define _BOARD_H_

#include "chess.h"

typedef struct _BOARD_ {
   BITBOARD pawns;
   BITBOARD knights;
   BITBOARD bishops;
   BITBOARD rooks;
   BITBOARD queens;
   BITBOARD kings;

   struct {
     BITBOARD whitepieces;
     BITBOARD blackpieces;
   } by_colour;

   BITBOARD en_passant;

   COLOUR next;
   CASTLE castle;

} BOARD;

#define COLOUR_BB(board, colour) (((BITBOARD*)(&(board->by_colour)))[colour])
#define NEXT_COLOUR_BB(board)    COLOUR_BB(board, board->next)
#define OCCUPANCY_BB(board)      (board->by_colour.whitepieces | board->by_colour.blackpieces)


BOARD * initial_board();
BOARD * parse_fen(const char * fen);
PIECE piece_at_board(const BOARD* board, BITBOARD bb);
COLOUR colour_at_board(const BOARD* board, SQUARE sq);
void print_board(const BOARD* board);
void print_fen(const BOARD* board);

int evaluate(const BOARD * board);

#endif /* ifndef _BOARD_H_ */
