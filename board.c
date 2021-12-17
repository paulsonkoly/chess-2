/*
bitboard representation & move generator
Copyright © 2021 Paul Sonkoly

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t SQUARE;
#define NO_SQUARE ((SQUARE)(-1))

typedef uint8_t PIECE;
typedef uint8_t COLOUR;

#define NO_PIECE ((PIECE)0)
#define PAWN     ((PIECE)1)
#define KNIGHT   ((PIECE)2)
#define BISHOP   ((PIECE)3)
#define ROOK     ((PIECE)4)
#define QUEEN    ((PIECE)5)
#define KING     ((PIECE)6)

#define WHITE ((COLOUR)0)
#define BLACK ((COLOUR)1)

typedef uint64_t BITBOARD;

#define BITBOARD_SCAN(bb) \
  for (BITBOARD __## bb ##__bb__ = bb; __## bb ##__bb__  > 0; __## bb ##__bb__ &= __## bb ##__bb__ - 1)

#define BITBOARD_SCAN_ITER(bb) (ffsl(__## bb ##__bb__) - 1)

#define NO_CASTLE    ((CASTLE)0)
#define SHORT_CASTLE ((CASTLE)1)
#define LONG_CASTLE  ((CASTLE)2)

#define CALC_CASTLE(col, cas) (cas << (2 * col))

#define WHITE_SHORT_CASTLE CALC_CASTLE(WHITE, SHORT_CASTLE)
#define WHITE_LONG_CASTLE  CALC_CASTLE(WHITE, LONG_CASTLE)
#define BLACK_SHORT_CASTLE CALC_CASTLE(BLACK, SHORT_CASTLE)
#define BLACK_LONG_CASTLE  CALC_CASTLE(BLACK, LONG_CASTLE)

#define ALL_CASTLES (WHITE_SHORT_CASTLE | WHITE_LONG_CASTLE | BLACK_SHORT_CASTLE | BLACK_LONG_CASTLE)

typedef uint8_t CASTLE;

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

   COLOUR next;
   SQUARE en_passant;
   CASTLE castle;

} BOARD;

#define COLOUR_BB(board, colour) (((BITBOARD*)(&(board->by_colour)))[colour])
#define NEXT_COLOUR_BB(board)    COLOUR_BB(board, board->next)
#define OCCUPANCY_BB(board)      (board->by_colour.whitepieces | board->by_colour.blackpieces)

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

PIECE piece_at_board(const BOARD* board, SQUARE sq) {
  BITBOARD bb = (BITBOARD)1 << sq;

  if (board->pawns & bb) return PAWN;
  if (board->knights & bb) return KNIGHT;
  if (board->bishops & bb) return BISHOP;
  if (board->rooks & bb) return ROOK;
  if (board->queens & bb) return QUEEN;
  if (board->kings & bb) return KING;

  return 0;
}

COLOUR colour_at_board(const BOARD* board, SQUARE sq) {
  BITBOARD bb = (BITBOARD)1 << sq;

  if (board->by_colour.whitepieces & bb) return WHITE;
  if (board->by_colour.blackpieces & bb) return BLACK;

  return 0;
}

void print_board(BOARD* board) {

  const char * s = " pnbrqkPNBRQK";

  for (SQUARE r = 56; r < 64; r -= 8) {
    for (SQUARE f = 0; f != 8; f += 1) {
      SQUARE sq = r | f;
      PIECE p = piece_at_board(board, sq);
      COLOUR c = colour_at_board(board, sq);

      printf("%c", s[6 * c + p]);
    }

    printf("\n");
  }
}

void print_fen(BOARD* board) {

  const char * s = " PNBRQKpnbrqk";
  int count = 0;

  for (SQUARE r = 56; r < 64; r -= 8) {
    for (SQUARE f = 0; f != 8; f += 1) {
      SQUARE sq = r | f;
      PIECE p = piece_at_board(board, sq);
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

  if (board->en_passant != NO_SQUARE) {
    SQUARE f = (board->en_passant & 7), r = (board->en_passant >> 3);

    printf(" %c%c 0 1 ", 'a' + f, '1' + r);
  }
  else {
    printf(" - 0 1 ");
  }
}

void print_bitboard(BITBOARD bb) {

  printf("--------\n");
  for (SQUARE r = 56; r < 64; r -= 8) {
    for (SQUARE f = 0; f != 8; f += 1) {
      SQUARE sq = r | f;

      printf("%c", bb & ((BITBOARD)1 << sq ) ? 'X' : '.');
    }
    printf("\n");
  }
  printf("--------\n");
}

#define IS_CASTLE  ((CASTLE)1 << 4)

typedef struct _MOVE_ {
  SQUARE from;
  SQUARE to;
  PIECE  piece;
  PIECE  promotion;
  /* 4 byte */
  SQUARE next_en_passant;
  CASTLE castle;
  uint8_t pad[2];
  /* 4 byte */
  BITBOARD en_passant;
} MOVE;

void print_move(BOARD * board, MOVE * move) {
  if (IS_CASTLE & move->castle) {
    CASTLE c = ALL_CASTLES & move->castle;
    const char * cs = "e1g1\0e1c1\0e8g8\0e8c8";

    c = (c >> 1) | (c >> 3) | ((c >> 3) << 1);

    printf("%s", &cs[c*5]);
  } else {
    SQUARE from = move->from;
    SQUARE to   = move->to;

    SQUARE ff = from & 7, fr = from >> 3, tf = to & 7, tr = to >> 3;

    printf("%c%c%c%c", 'a' + ff, '1' + fr, 'a' + tf, '1' + tr);

    if (move->promotion) {
      const char * p = "  kbrq";

      printf("%c", p[move->promotion]);
    }
  }
  /* else { */
  /*   const char * ps = "  NBRQK"; */
  /*   const char * fs = "abcdefgh"; */

  /*   if (OCCUPANCY_BB(board) & ((BITBOARD)1 << move->to)) { */
  /*     if (move->piece == PAWN) { */
  /*       printf("%cx%c%d", fs[move->from % 8], fs[move->to % 8], 1 + move->to / 8); */
  /*     } */
  /*     else { */
  /*       printf("%cx%c%d", ps[move->piece], fs[move->to % 8], 1 + move->to / 8); */
  /*     } */
  /*   } */
  /*   else { */
  /*     printf("%c%c%d", ps[move->piece], fs[move->to % 8], 1 + move->to / 8); */
  /*   } */
  /* } */
}

static const BITBOARD knight_attacks[] = {
  0x0000000000020400, 0x0000000000050800, 0x00000000000a1100, 0x0000000000142200,
  0x0000000000284400, 0x0000000000508800, 0x0000000000a01000, 0x0000000000402000,
  0x0000000002040004, 0x0000000005080008, 0x000000000a110011, 0x0000000014220022,
  0x0000000028440044, 0x0000000050880088, 0x00000000a0100010, 0x0000000040200020,
  0x0000000204000402, 0x0000000508000805, 0x0000000a1100110a, 0x0000001422002214,
  0x0000002844004428, 0x0000005088008850, 0x000000a0100010a0, 0x0000004020002040,
  0x0000020400040200, 0x0000050800080500, 0x00000a1100110a00, 0x0000142200221400,
  0x0000284400442800, 0x0000508800885000, 0x0000a0100010a000, 0x0000402000204000,
  0x0002040004020000, 0x0005080008050000, 0x000a1100110a0000, 0x0014220022140000,
  0x0028440044280000, 0x0050880088500000, 0x00a0100010a00000, 0x0040200020400000,
  0x0204000402000000, 0x0508000805000000, 0x0a1100110a000000, 0x1422002214000000,
  0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000,
  0x0400040200000000, 0x0800080500000000, 0x1100110a00000000, 0x2200221400000000,
  0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000,
  0x0004020000000000, 0x0008050000000000, 0x00110a0000000000, 0x0022140000000000,
  0x0044280000000000, 0x0088500000000000, 0x0010a00000000000, 0x0020400000000000,
};

MOVE * add_knight_moves(BOARD * board, MOVE * move) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD knights = board->knights & colour;

  BITBOARD_SCAN(knights) {
    SQUARE from = BITBOARD_SCAN_ITER(knights);
    BITBOARD t = knight_attacks[from] & ~ colour;

    BITBOARD_SCAN(t) {
      SQUARE to = BITBOARD_SCAN_ITER(t);

      move->from            = from;
      move->to              = to;
      move->piece           = KNIGHT;
      move->promotion       = NO_PIECE;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;

      move++;
    }
  }

  return move;
}

BITBOARD bishop_attacks[64][512];
BITBOARD rook_attacks[64][4096];

static const BITBOARD bishop_masks[64] = {
  0x0040201008040200, 0x0000402010080400, 0x0000004020100a00, 0x0000000040221400,
  0x0000000002442800, 0x0000000204085000, 0x0000020408102000, 0x0002040810204000,
  0x0020100804020000, 0x0040201008040000, 0x00004020100a0000, 0x0000004022140000,
  0x0000000244280000, 0x0000020408500000, 0x0002040810200000, 0x0004081020400000,
  0x0010080402000200, 0x0020100804000400, 0x004020100a000a00, 0x0000402214001400,
  0x0000024428002800, 0x0002040850005000, 0x0004081020002000, 0x0008102040004000,
  0x0008040200020400, 0x0010080400040800, 0x0020100a000a1000, 0x0040221400142200,
  0x0002442800284400, 0x0004085000500800, 0x0008102000201000, 0x0010204000402000,
  0x0004020002040800, 0x0008040004081000, 0x00100a000a102000, 0x0022140014224000,
  0x0044280028440200, 0x0008500050080400, 0x0010200020100800, 0x0020400040201000,
  0x0002000204081000, 0x0004000408102000, 0x000a000a10204000, 0x0014001422400000,
  0x0028002844020000, 0x0050005008040200, 0x0020002010080400, 0x0040004020100800,
  0x0000020408102000, 0x0000040810204000, 0x00000a1020400000, 0x0000142240000000,
  0x0000284402000000, 0x0000500804020000, 0x0000201008040200, 0x0000402010080400,
  0x0002040810204000, 0x0004081020400000, 0x000a102040000000, 0x0014224000000000,
  0x0028440200000000, 0x0050080402000000, 0x0020100804020000, 0x0040201008040200,
};

static const BITBOARD rook_masks[64] = {
  0x000101010101017e, 0x000202020202027c, 0x000404040404047a, 0x0008080808080876,
  0x001010101010106e, 0x002020202020205e, 0x004040404040403e, 0x008080808080807e,
  0x0001010101017e00, 0x0002020202027c00, 0x0004040404047a00, 0x0008080808087600,
  0x0010101010106e00, 0x0020202020205e00, 0x0040404040403e00, 0x0080808080807e00,
  0x00010101017e0100, 0x00020202027c0200, 0x00040404047a0400, 0x0008080808760800,
  0x00101010106e1000, 0x00202020205e2000, 0x00404040403e4000, 0x00808080807e8000,
  0x000101017e010100, 0x000202027c020200, 0x000404047a040400, 0x0008080876080800,
  0x001010106e101000, 0x002020205e202000, 0x004040403e404000, 0x008080807e808000,
  0x0001017e01010100, 0x0002027c02020200, 0x0004047a04040400, 0x0008087608080800,
  0x0010106e10101000, 0x0020205e20202000, 0x0040403e40404000, 0x0080807e80808000,
  0x00017e0101010100, 0x00027c0202020200, 0x00047a0404040400, 0x0008760808080800,
  0x00106e1010101000, 0x00205e2020202000, 0x00403e4040404000, 0x00807e8080808000,
  0x007e010101010100, 0x007c020202020200, 0x007a040404040400, 0x0076080808080800,
  0x006e101010101000, 0x005e202020202000, 0x003e404040404000, 0x007e808080808000,
  0x7e01010101010100, 0x7c02020202020200, 0x7a04040404040400, 0x7608080808080800,
  0x6e10101010101000, 0x5e20202020202000, 0x3e40404040404000, 0x7e80808080808000,
};

static const SQUARE bishop_shifts[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

static const SQUARE rook_shifts[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

static const BITBOARD bishop_magics[64] = {
  0x01442b1002020240, 0x23280a482202e891, 0x685843810202408c, 0x287820d044d95020,
  0x5a6b1041408282c3, 0x01482c0c60008813, 0x024a43083841f042, 0x24a0460150080c00,
  0x4602412408220640, 0x2300a00142420042, 0x0100281806408228, 0x41100405020c1c62,
  0x03053a0210002808, 0x0400a202302c4465, 0x054504012c022018, 0x000a908405051012,
  0x0c600c901403a800, 0x082202e018910103, 0x529000260c007160, 0x1004041824049080,
  0x2a0400b82208020a, 0x0a42c0020100a000, 0x10070024040b5461, 0x4031011c49082111,
  0x43d0c12408820400, 0x0844549006082810, 0x2000680230018264, 0x000a018008008042,
  0x4481011109004000, 0x451014800c0a010a, 0x41180350408a2802, 0x1163020001028082,
  0x1108200400281820, 0x0622312423101031, 0x4022a0f000680488, 0x0991404800018200,
  0x280508060001a200, 0x3b821202023c4802, 0x00011e1a00040100, 0x089112020483a304,
  0x0804211828874060, 0x14c209842428a002, 0x0042010041041822, 0x40900c2104002040,
  0x1020441d04080609, 0x40844810010010c8, 0x2028231122042c0a, 0x40508200410428c1,
  0x1424473011102926, 0x0518410c10064a30, 0x04b1824044501029, 0x204051c120880641,
  0x5012f41060622031, 0x2875604722460049, 0x61a0202409829031, 0x400714080481000c,
  0x401201041a024221, 0x62140200421e1004, 0x009b828826051057, 0x2440370004209814,
  0x404601a741028600, 0x004fd42024091201, 0x262c888a14444400, 0x4208652400820202,
};

static const BITBOARD rook_magics[64] = {
  0x0200128102002041, 0x64c0002002100440, 0x0100200008c05102, 0x0200120020043840,
  0x1b00030008008410, 0x4200120054211008, 0x24000d5000880a04, 0x610000270000c886,
  0x06ca801040022580, 0x0c0040045000a001, 0x0021001102c06000, 0x08d1001000a19b00,
  0x2b17005800841100, 0x1206000201300894, 0x3004000813041012, 0x681a00056106008c,
  0x2d60628002c00483, 0x0020014000505012, 0x4026848020029000, 0x010452000862c200,
  0x1308818008002400, 0x0c1b080120104004, 0x1089240010863108, 0x0211320004008051,
  0x2000800300210842, 0x0620810600220ac1, 0x0404420200201482, 0x0042500300096100,
  0x2801a21200093200, 0x2c12000600043019, 0x4042000200580479, 0x40040d02000a815c,
  0x2900804000800030, 0x0303420102002c83, 0x004200a142003084, 0x2660520062000940,
  0x026200a00a001084, 0x3802001d42001830, 0x0808662104001088, 0x110222a52a0003a4,
  0x00d88b2140028007, 0x735950002000400b, 0x34c5820020160040, 0x0206003820420011,
  0x684a002830c60020, 0x405e001410a20009, 0x0440221011040008, 0x211009014096000c,
  0x129c421502208200, 0x6a42042052850200, 0x2f09014a6002f100, 0x0a06191042012200,
  0x58f0715500780100, 0x090201904c081200, 0x310e002528041200, 0x484da14408810200,
  0x4d3a416133008001, 0x4406802436400101, 0x0043c17912200101, 0x1240615000890015,
  0x4c4600302004c822, 0x0122001008410462, 0x04be821803102084, 0x5202884c0500826a,
};

BITBOARD calculate_bishop_attack(SQUARE sq, BITBOARD occ) {
  BITBOARD result = 0;

  int8_t r = sq / 8, f = sq % 8, rr, ff;

  for (rr = r + 1, ff = f + 1; rr <= 7 && ff <= 7; rr++, ff++) {
    result |= ((BITBOARD)1 << (ff + rr * 8));
    if (occ & ((BITBOARD)1 << (ff + rr * 8))) break;
  }
  for (rr = r + 1, ff = f - 1; rr <= 7 && ff >= 0; rr++, ff--) {
    result |= ((BITBOARD)1 << (ff + rr * 8));
    if (occ & ((BITBOARD)1 << (ff + rr * 8))) break;
  }
  for (rr = r - 1, ff = f + 1; rr >= 0 && ff <= 7; rr--, ff++) {
    result |= ((BITBOARD)1 << (ff + rr * 8));
    if (occ & ((BITBOARD)1 << (ff + rr * 8))) break;
  }
  for (rr = r - 1, ff = f - 1; rr >= 0 && ff >= 0; rr--, ff--) {
    result |= ((BITBOARD)1 << (ff + rr * 8));
    if (occ & ((BITBOARD)1 << (ff + rr * 8))) break;
  }

  return result;
}

BITBOARD calculate_rook_attack(SQUARE sq, BITBOARD occ) {
  BITBOARD result = 0;

  int8_t r = sq / 8, f = sq % 8, rr, ff;

  for (rr = r + 1; rr <= 7; rr++) {
    result |= ((BITBOARD)1 << (f + rr * 8));
    if (occ & ((BITBOARD)1 << (f + rr * 8))) break;
  }
  for (rr = r - 1; rr >= 0; rr--) {
    result |= ((BITBOARD)1 << (f + rr * 8));
    if (occ & ((BITBOARD)1 << (f + rr * 8))) break;
  }
  for (ff = f + 1; ff <= 7; ff++) {
    result |= ((BITBOARD)1 << (ff + r * 8));
    if (occ & ((BITBOARD)1 << (ff + r * 8))) break;
  }
  for (ff = f - 1; ff >= 0; ff--) {
    result |= ((BITBOARD)1 << (ff + r * 8));
    if (occ & ((BITBOARD)1 << (ff + r * 8))) break;
  }

  return result;
}

void initialize_magic() {
  for (SQUARE sq = 0; sq < 64; sq++) {
    BITBOARD mask  = bishop_masks[sq];
    BITBOARD magic = bishop_magics[sq];
    SQUARE shift   = bishop_shifts[sq];
    BITBOARD occ   = mask;

    /* ripple bits for occupancy in the mask */
    while (1) {
      BITBOARD attack = calculate_bishop_attack(sq, occ);
      bishop_attacks[sq][(occ * magic) >> (64 - shift)] = attack;

      occ = (occ - mask) & mask;
      if (occ == mask) break;
    }
  }

  for (SQUARE sq = 0; sq < 64; sq++) {
    BITBOARD mask  = rook_masks[sq];
    BITBOARD magic = rook_magics[sq];
    SQUARE shift   = rook_shifts[sq];
    BITBOARD occ   = mask;

    /* ripple bits for occupancy in the mask */
    while (1) {
      BITBOARD attack = calculate_rook_attack(sq, occ);
      rook_attacks[sq][(occ * magic) >> (64 - shift)] = attack;

      occ = (occ - mask) & mask;
      if (occ == mask) break;
    }
  }
}

static inline BITBOARD bishop_bitboard(BOARD * board, SQUARE sq) {
  BITBOARD mask  = bishop_masks[sq];
  BITBOARD magic = bishop_magics[sq];
  BITBOARD occ   = OCCUPANCY_BB(board) & mask;
  SQUARE shift   = bishop_shifts[sq];

  return bishop_attacks[sq][(occ * magic) >> (64 - shift)];
}

static inline BITBOARD rook_bitboard(BOARD * board, SQUARE sq) {
  BITBOARD mask  = rook_masks[sq];
  BITBOARD magic = rook_magics[sq];
  BITBOARD occ   = OCCUPANCY_BB(board) & mask;
  SQUARE shift   = rook_shifts[sq];

  return rook_attacks[sq][(occ * magic) >> (64 - shift)];
}

MOVE * add_bishop_moves(BOARD * board, MOVE * move) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD bishops = board->bishops & colour;

  BITBOARD_SCAN(bishops) {
    SQUARE from = BITBOARD_SCAN_ITER(bishops);
    BITBOARD t = bishop_bitboard(board, from) & ~colour;

    BITBOARD_SCAN(t) {
      SQUARE to = BITBOARD_SCAN_ITER(t);

      move->from            = from;
      move->to              = to;
      move->piece           = BISHOP;
      move->promotion       = NO_PIECE;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;

      move++;
    }
  }

  return move;
}

MOVE * add_rook_moves(BOARD * board, MOVE * move) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD rooks = board->rooks & colour;

  BITBOARD_SCAN(rooks) {
    SQUARE from = BITBOARD_SCAN_ITER(rooks);
    BITBOARD t = rook_bitboard(board, from) & ~colour;

    BITBOARD_SCAN(t) {
      SQUARE   to = BITBOARD_SCAN_ITER(t);
      BITBOARD castle = ((BITBOARD)1 << from);

      move->from            = from;
      move->to              = to;
      move->piece           = ROOK;
      move->promotion       = NO_PIECE;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;

      castle =
        (castle & ((BITBOARD)1 << 0) << 1)   |
        (castle & ((BITBOARD)1 << 7) >> 6)   |
        (castle & ((BITBOARD)1 << 56) >> 52) |
        (castle & ((BITBOARD)1 << 63) >> 60);

      move->castle = castle;

      move++;
    }
  }

  return move;
}

MOVE * add_queen_moves(BOARD * board, MOVE * move) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD queens = board->queens & colour;

  BITBOARD_SCAN(queens) {
    SQUARE from = BITBOARD_SCAN_ITER(queens);

    BITBOARD t = (bishop_bitboard(board, from) | rook_bitboard(board, from)) & ~colour;

    BITBOARD_SCAN(t) {
      SQUARE to = BITBOARD_SCAN_ITER(t);

      move->from            = from;
      move->to              = to;
      move->piece           = QUEEN;
      move->promotion       = NO_PIECE;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;

      move++;
    }
  }

  return move;
}

#define SINGLE_PAWN_PUSH(colour, bitboard) (((bitboard) << 8) >> ((colour) << 4))

BITBOARD single_pawn_pushes(BITBOARD pawns, BITBOARD empty, COLOUR colour) {
  return SINGLE_PAWN_PUSH(colour, pawns) & empty;
}

static const BITBOARD double_pawn_push_ranks[2] = {
  0x00000000ff000000, 0x000000ff00000000
};

BITBOARD double_pawn_pushes(BITBOARD pawns, BITBOARD empty, COLOUR colour) {
  BITBOARD single_pushes = single_pawn_pushes(pawns, empty, colour);

  return SINGLE_PAWN_PUSH(colour,single_pushes) & empty & double_pawn_push_ranks[colour];
}

static const BITBOARD pawn_capture_files[2] = {
  0x7f7f7f7f7f7f7f7f, 0xfefefefefefefefe
};

BITBOARD pawn_captures(BITBOARD pawns, BITBOARD opp, COLOUR colour) {
  BITBOARD left_captures  = (SINGLE_PAWN_PUSH(colour, pawns) & pawn_capture_files[0]) << 1;
  BITBOARD right_captures = (SINGLE_PAWN_PUSH(colour, pawns) & pawn_capture_files[1]) >> 1;

  return (left_captures | right_captures) & opp;
}

#define PROMOTIONS ((BITBOARD)0xff000000000000ff)

MOVE * add_pawn_moves(BOARD * board, MOVE * move) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD opp    = COLOUR_BB(board, 1 - board->next);
  BITBOARD pawns  = board->pawns & colour;
  BITBOARD empty  = ~ OCCUPANCY_BB(board);
  BITBOARD s      = single_pawn_pushes(pawns, empty, board->next);
  BITBOARD t;

  t = s & ~PROMOTIONS;
  BITBOARD_SCAN(t) {
    SQUARE to = BITBOARD_SCAN_ITER(t);
    SQUARE from = to + 8 * (2 * board->next - 1);

    move->from            = from;
    move->to              = to;
    move->piece           = PAWN;
    move->promotion       = NO_PIECE;
    move->en_passant      = 0;
    move->next_en_passant = NO_SQUARE;
    move->castle          = NO_CASTLE;

    move++;
  }

  t = s & PROMOTIONS;
  BITBOARD_SCAN(t) {
    SQUARE to = BITBOARD_SCAN_ITER(t);
    SQUARE from = to + 8 * (2 * board->next - 1);

    for (PIECE piece = QUEEN; piece > PAWN; --piece) {
      move->from            = from;
      move->to              = to;
      move->piece           = PAWN;
      move->promotion       = piece;
      move->en_passant      = 0;
      move->next_en_passant = NO_SQUARE;
      move->castle          = NO_CASTLE;

      move++;
    }
  }

  t = double_pawn_pushes(pawns, empty, board->next);

  BITBOARD_SCAN(t) {
    SQUARE to = BITBOARD_SCAN_ITER(t);
    SQUARE from = to + 16 * (2 * board->next - 1);

    move->from            = from;
    move->to              = to;
    move->piece           = PAWN;
    move->promotion       = NO_PIECE;
    move->en_passant      = 0;
    move->next_en_passant = (from + to) / 2;
    move->castle          = NO_CASTLE;

    move++;
  }

  {
    BITBOARD en_passant = board->en_passant == NO_SQUARE ? 0 : (BITBOARD)1 << board->en_passant;
    BITBOARD c = pawn_captures(pawns, opp | en_passant, board->next);

    t = c & ~PROMOTIONS;
    BITBOARD_SCAN(t) {
      SQUARE to = BITBOARD_SCAN_ITER(t);
      BITBOARD capture = (BITBOARD)1 << to;
      BITBOARD pawnbb = pawn_captures(capture, pawns, 1 - board->next);

      BITBOARD_SCAN(pawnbb) {
        SQUARE from = BITBOARD_SCAN_ITER(pawnbb);

        move->from            = from;
        move->to              = to;
        move->piece           = PAWN;
        move->promotion       = NO_PIECE;
        move->en_passant      = en_passant & capture;
        move->next_en_passant = NO_SQUARE;
        move->castle          = NO_CASTLE;

        move++;
      }
    }

    t = c & PROMOTIONS;
    BITBOARD_SCAN(t) {
      SQUARE to = BITBOARD_SCAN_ITER(t);
      BITBOARD capture = (BITBOARD)1 << to;
      BITBOARD pawnbb;

      if (to >= 56) {
        /* TODO we should avoid conditional if we could but assumption is that promotions are very rare so shouldn't 
         * hurt performance
         */
        pawnbb = pawns &
          ((((capture & pawn_capture_files[0]) << 1) >> 8) | (((capture & pawn_capture_files[1]) >> 1) >> 8));
      } else
        pawnbb = pawn_captures(capture, pawns, 1 - board->next);

      BITBOARD_SCAN(pawnbb) {
        SQUARE from = BITBOARD_SCAN_ITER(pawnbb);

        for (PIECE piece = QUEEN; piece > PAWN; --piece) {
          move->from            = from;
          move->to              = to;
          move->piece           = PAWN;
          move->promotion       = piece;
          move->en_passant      = 0;
          move->next_en_passant = NO_SQUARE;
          move->castle          = NO_CASTLE;

          move++;
        }
      }
    }
  }

  return move;
}

static const BITBOARD king_attacks[] = {
  0x0000000000000302, 0x0000000000000705, 0x0000000000000e0a, 0x0000000000001c14,
  0x0000000000003828, 0x0000000000007050, 0x000000000000e0a0, 0x000000000000c040,
  0x0000000000030203, 0x0000000000070507, 0x00000000000e0a0e, 0x00000000001c141c,
  0x0000000000382838, 0x0000000000705070, 0x0000000000e0a0e0, 0x0000000000c040c0,
  0x0000000003020300, 0x0000000007050700, 0x000000000e0a0e00, 0x000000001c141c00,
  0x0000000038283800, 0x0000000070507000, 0x00000000e0a0e000, 0x00000000c040c000,
  0x0000000302030000, 0x0000000705070000, 0x0000000e0a0e0000, 0x0000001c141c0000,
  0x0000003828380000, 0x0000007050700000, 0x000000e0a0e00000, 0x000000c040c00000,
  0x0000030203000000, 0x0000070507000000, 0x00000e0a0e000000, 0x00001c141c000000,
  0x0000382838000000, 0x0000705070000000, 0x0000e0a0e0000000, 0x0000c040c0000000,
  0x0003020300000000, 0x0007050700000000, 0x000e0a0e00000000, 0x001c141c00000000,
  0x0038283800000000, 0x0070507000000000, 0x00e0a0e000000000, 0x00c040c000000000,
  0x0302030000000000, 0x0705070000000000, 0x0e0a0e0000000000, 0x1c141c0000000000,
  0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000,
  0x0203000000000000, 0x0507000000000000, 0x0a0e000000000000, 0x141c000000000000,
  0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000,
};

int in_check(BOARD * board) {
  BITBOARD opp    = COLOUR_BB(board, 1 - board->next);
  BITBOARD king   = board->kings & NEXT_COLOUR_BB(board);
  SQUARE sq       = ffsl(king) - 1;

  if (king_attacks[sq] & opp & board->kings) {
    return 1;
  }

  if (knight_attacks[sq] & opp & board->knights) {
    return 1;
  }

  if (bishop_bitboard(board, sq) & opp & board->bishops) {
    return 1;
  }

  if (rook_bitboard(board, sq) & opp & board->rooks) {
    return 1;
  }

  if ((bishop_bitboard(board, sq) | rook_bitboard(board, sq)) & opp & board->queens) {
    return 1;
  }

  if (pawn_captures(king, opp & board->pawns, board->next)) {
    return 1;
  }

  return 0;
}

MOVE * add_king_moves(BOARD * board, MOVE * move) {
  BITBOARD colour = NEXT_COLOUR_BB(board);
  BITBOARD king = board->kings & colour;
  SQUARE from = ffsl(king) - 1;
  BITBOARD t = king_attacks[from] & ~ colour;

  BITBOARD_SCAN(t) {
    SQUARE to = BITBOARD_SCAN_ITER(t);

    move->from            = from;
    move->to              = to;
    move->piece           = KING;
    move->promotion       = NO_PIECE;
    move->en_passant      = 0;
    move->next_en_passant = NO_SQUARE;
    move->castle          = (SHORT_CASTLE | LONG_CASTLE) << (board->next * 2);

    move++;
  }

  return move;
}

static const BITBOARD castle_squares[4] = {
  0x0000000000000060, 0x000000000000000e,
  0x6000000000000000, 0x0e00000000000000
};

MOVE * add_castles(BOARD * board, MOVE * move) {
  BITBOARD occ = OCCUPANCY_BB(board);

  if (in_check(board)) {
    return move;
  }

  for (CASTLE castle = 0; castle <= 1; ++castle) {
    CASTLE c = 2 * board->next + castle;

    if (board->castle & ((CASTLE)1 << c)) {
      if (!(castle_squares[c] & occ)) {
        move->castle = IS_CASTLE | c;

        move++;
      }
    }
  }

  return move;
}

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

void execute_move(BOARD * board, MOVE * move) {
  BITBOARD * my = (BITBOARD*)&board->by_colour + board->next;

  if (move->castle & IS_CASTLE) {
    CASTLE c = move->castle & ALL_CASTLES;
    /* 4 bit one hot to 2 complement */
    c = (c >> 1) | (c >> 3) | ((c >> 3) << 1);
    BITBOARD king_flip = castle_king_flip[c];
    BITBOARD rook_flip = castle_rook_flip[c];

    board->kings ^= king_flip;
    board->rooks ^= rook_flip;
    *my ^= (king_flip | rook_flip);

    board->en_passant = NO_SQUARE;
    board->castle &= ~((SHORT_CASTLE + 2 * board->next) | (LONG_CASTLE + 2 * board->next));

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
    board->castle &= ~move->castle;
  }

#define DEBUG_MOVES
#ifdef DEBUG_MOVES
#define EXPECT_EQL(ba, bb)                                                           \
if ((ba) != (bb)) {                                                                  \
  printf("BOARD INCONSISTENCY - " #ba " != " #bb " %016lx != %016lx\n", (BITBOARD)ba, (BITBOARD)bb); \
  print_board(board);                                                                \
  print_move(board, move);                                                           \
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

unsigned long long perft(BOARD * board, int depth, int print) {
  MOVE moves[60];
  MOVE * moveptr = moves;
  BOARD copy;
  unsigned long long int count = 0;

  if (depth == 0) {
    return 1;
  }

  moveptr = add_knight_moves(board, moveptr);
  moveptr = add_king_moves(board, moveptr);
  moveptr = add_bishop_moves(board, moveptr);
  moveptr = add_rook_moves(board, moveptr);
  moveptr = add_queen_moves(board, moveptr);
  moveptr = add_pawn_moves(board, moveptr);
  moveptr = add_castles(board, moveptr);

  for (MOVE * ptr = moves; ptr != moveptr; ptr++) {
    copy = *board;
    unsigned long long int current;

    execute_move(&copy, ptr);

    if (in_check(&copy)) {
      continue;
    }

    copy.next = 1 - copy.next;

    if (print) print_fen(&copy);

    current = perft(&copy, depth - 1, 0);

    if (print) {
      print_move(board, ptr);
      printf(" %lld\n", current);
    }

    count += current;
  }

  return count;
}

int main(int argc, const char * argv[]) {
  BOARD * b;
  int depth;

  initialize_magic();

  if (argc == 1) {
    b = initial_board();
    depth = 3;
  } else {
    sscanf(argv[1], "%d", &depth);
    b = parse_fen(argv[2]);
  }

  printf("%lld\n", perft(b, depth, 1));

  free(b);

  return 0;
}

