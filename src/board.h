#ifndef _BOARD_H_
#define _BOARD_H_

#include "chess.h"
#include "zobrist.h"

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

   int halfmovecnt;
   struct {
     HASH hash;
#define HIST_CANT_REPEAT 0x00000001
     unsigned int flags;
   } history[MAX_GAME_PLIES];

} BOARD;

#define COLOUR_BB(board, colour) (((BITBOARD*)(&(board->by_colour)))[colour])
#define NEXT_COLOUR_BB(board)    COLOUR_BB(board, board->next)
#define OCCUPANCY_BB(board)      (board->by_colour.whitepieces | board->by_colour.blackpieces)


BOARD * initial_board();
BOARD * parse_fen(const char * fen);
void play_uci_moves(BOARD * board, const char * moves);
PIECE piece_at_board(const BOARD* board, BITBOARD bb);
COLOUR colour_at_board(const BOARD* board, SQUARE sq);
void print_board(const BOARD* board);
void print_fen(const BOARD* board);
HASH calculate_hash(const BOARD* board);

#endif /* ifndef _BOARD_H_ */
