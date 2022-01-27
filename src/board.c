#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "board.h"
#include "move.h"
#include "movegen.h"
#include "moveexec.h"

BOARD * initial_board() {
  BOARD * board;

  if (NULL == (board = malloc(sizeof(BOARD)))) {
    return NULL;
  }

  board->pawns       = 0x00ff00000000ff00;
  board->knights     = 0x4200000000000042;
  board->bishops     = 0x2400000000000024;
  board->rooks       = 0x8100000000000081;
  board->queens      = 0x0800000000000008;
  board->kings       = 0x1000000000000010;

  board->by_colour.whitepieces = 0x000000000000ffff;
  board->by_colour.blackpieces = 0xffff000000000000;

  board->next = WHITE;
  board->en_passant = NO_SQUARE;
  board->castle = ALL_CASTLES;

  return board;
}

BOARD * parse_fen(const char * fen) {
  BOARD * board;

  SQUARE f = 0;
  SQUARE r = 7;
  const char * ptr;

  int finished = 0;

  if (NULL == (board = malloc(sizeof(BOARD)))) {
    return NULL;
  }

  memset(board, 0, sizeof(BOARD));

  for (ptr = fen; *ptr && !finished; ++ptr) {
    BITBOARD flag = ((BITBOARD)1 << ((8 * r) + f));
    switch (*ptr) {
      case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
        f += *ptr - '0';
        break;
      case '/':
        f = 0;
        r -= 1;
        break;
      case 'R': board->rooks   |= flag; board->by_colour.whitepieces |= flag; f++; break;
      case 'r': board->rooks   |= flag; board->by_colour.blackpieces |= flag; f++; break;
      case 'N': board->knights |= flag; board->by_colour.whitepieces |= flag; f++; break;
      case 'n': board->knights |= flag; board->by_colour.blackpieces |= flag; f++; break;
      case 'B': board->bishops |= flag; board->by_colour.whitepieces |= flag; f++; break;
      case 'b': board->bishops |= flag; board->by_colour.blackpieces |= flag; f++; break;
      case 'Q': board->queens  |= flag; board->by_colour.whitepieces |= flag; f++; break;
      case 'q': board->queens  |= flag; board->by_colour.blackpieces |= flag; f++; break;
      case 'K': board->kings   |= flag; board->by_colour.whitepieces |= flag; f++; break;
      case 'k': board->kings   |= flag; board->by_colour.blackpieces |= flag; f++; break;
      case 'P': board->pawns   |= flag; board->by_colour.whitepieces |= flag; f++; break;
      case 'p': board->pawns   |= flag; board->by_colour.blackpieces |= flag; f++; break;
      default: finished = 1;
    }
  }
  for (; *ptr == ' '; ++ptr);

  switch (*ptr++) {
    case 'w': board->next = WHITE; break;
    case 'b': board->next = BLACK; break;
  }
  for (; *ptr == ' '; ++ptr);

  for (; *ptr != ' '; ++ptr) {
    switch (*ptr) {
      case 'K': board->castle |= CALC_CASTLE(WHITE, SHORT_CASTLE); break;
      case 'Q': board->castle |= CALC_CASTLE(WHITE, LONG_CASTLE); break;
      case 'k': board->castle |= CALC_CASTLE(BLACK, SHORT_CASTLE); break;
      case 'q': board->castle |= CALC_CASTLE(BLACK, LONG_CASTLE); break;
    }
  }
  for (; *ptr == ' '; ++ptr);

  if (*ptr != '-') {
    f = *ptr++ - 'a';
    r = *ptr - '1';
    board->en_passant = r * 8 + f;
  }

  return board;
}

void play_uci_moves(BOARD * board, const char * moves) {
  MOVE move;

  for (; *moves == ' '; ++moves);

  while ((*moves) != '\n') {
    PIECE promotion = NO_PIECE;
    SQUARE ff, fr, tf, tr;
    BITBOARD from;
    BITBOARD to;
    BITBOARD special = 0;
    PIECE piece;
    PIECE capture;
    CASTLE castle = NO_CASTLE;
    BITBOARD en_passant = board->en_passant;

    ff = *moves     - 'a';
    fr = *(moves+1) - '1';
    tf = *(moves+2) - 'a';
    tr = *(moves+3) - '1';

    from = (BITBOARD)1 << (fr * 8 + ff);
    to   = (BITBOARD)1 << (tr * 8 + tf);

    switch (*(moves + 4)) {
      case 'q': promotion = QUEEN;  break;
      case 'r': promotion = ROOK;   break;
      case 'b': promotion = BISHOP; break;
      case 'n': promotion = KNIGHT; break;
    }

    piece = piece_at_board(board, from);
    capture = piece_at_board(board, to);

    if (piece == PAWN && ((fr - tr) == 2 || (tr - fr) == 2)) {
      SQUARE epr = (fr + tr) / 2;

      en_passant |= ((BITBOARD)1 << (epr * 8 + ff));
    }

    if (promotion != NO_PIECE) {
      special = to;
    } else if (piece == PAWN && ff != tf && capture == NO_PIECE) {
      special = ((BITBOARD)1 << (fr * 8 + tf));
      capture = PAWN;
    } else if (piece == KING && ((ff - tf) == 2 || (tf - ff) == 2)) {
      unsigned index = ((fr & BLACK) << 1) | (ff < tf ? 0 : 1);

      special = castling_rook_from_to(index);
      castle = IS_CASTLE;
    }

    move.from            = from;
    move.to              = to;
    move.special         = special;
    move.piece           = piece;
    move.capture         = capture;
    move.promotion       = promotion;
    move.en_passant      = en_passant;
    move.castle          = castle | castle_update(board, piece, from | to);

    execute_move(board, & move);

    moves += (promotion == NO_PIECE ? 4 : 5);

    for (; *moves == ' '; ++moves);
  }
}

PIECE piece_at_board(const BOARD* board, BITBOARD bb) {
  if (board->pawns & bb) return PAWN;
  if (board->knights & bb) return KNIGHT;
  if (board->bishops & bb) return BISHOP;
  if (board->rooks & bb) return ROOK;
  if (board->queens & bb) return QUEEN;
  if (board->kings & bb) return KING;

  return NO_PIECE;
}

COLOUR colour_at_board(const BOARD* board, SQUARE sq) {
  BITBOARD bb = (BITBOARD)1 << sq;

  if (board->by_colour.whitepieces & bb) return WHITE;
  if (board->by_colour.blackpieces & bb) return BLACK;

  return 0;
}

void print_board(const BOARD* board) {

  const char * s = " pnbrqkPNBRQK";

  if (board->castle & CALC_CASTLE(WHITE, SHORT_CASTLE)) printf("K");
  if (board->castle & CALC_CASTLE(WHITE, LONG_CASTLE)) printf("Q");
  if (board->castle & CALC_CASTLE(BLACK, SHORT_CASTLE)) printf("k");
  if (board->castle & CALC_CASTLE(BLACK, LONG_CASTLE)) printf("q");
  printf("\n");

  for (SQUARE r = 56; r < 64; r -= 8) {
    for (SQUARE f = 0; f != 8; f += 1) {
      SQUARE sq = r | f;
      PIECE p = piece_at_board(board, 1ULL << sq);
      COLOUR c = colour_at_board(board, sq);

      printf("%c", s[6 * c + p]);
    }

    printf("\n");
  }
}

void print_fen(const BOARD* board) {

  const char * s = " PNBRQKpnbrqk";
  int count = 0;

  for (SQUARE r = 56; r < 64; r -= 8) {
    for (SQUARE f = 0; f != 8; f += 1) {
      SQUARE sq = r | f;
      PIECE p = piece_at_board(board, 1ULL << sq);
      COLOUR c = colour_at_board(board, sq);

      if (p) {
        if (count) {
          printf("%d", count);
          count = 0;
        }

        printf("%c", s[6 * c + p]);
      }
      else count++;
    }

    if (count) {
      printf("%d", count);
      count = 0;
    }
    if (r != 0) printf("/");
  }

  printf(" %c ", "wb"[board->next]);

  if (board->castle & CALC_CASTLE(WHITE, SHORT_CASTLE)) printf("K");
  if (board->castle & CALC_CASTLE(WHITE, LONG_CASTLE)) printf("Q");
  if (board->castle & CALC_CASTLE(BLACK, SHORT_CASTLE)) printf("k");
  if (board->castle & CALC_CASTLE(BLACK, LONG_CASTLE)) printf("q");

  if (board->en_passant) {
    SQUARE ep = ffsl(board->en_passant) - 1;
    SQUARE f = (ep & 7), r = (ep >> 3);

    printf(" %c%c 0 1 ", 'a' + f, '1' + r);
  }
  else {
    printf(" - 0 1 ");
  }
}

