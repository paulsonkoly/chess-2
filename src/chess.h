#ifndef _CHESS_H_
#define _CHESS_H_

#include <stdint.h>

#ifndef NDEBUG
#define DEBUG_ENABLE (debug = 1)
#define DEBUG 1
#define DEBUG_PRINT(...)                      \
  if (debug) {                                \
    printf(__VA_ARGS__);                      \
  }
extern int debug;
#define DEBUG_DISABLE (debug = 0)
#else
#define DEBUG_ENABLE
#define DEBUG_PRINT(...)
#define DEBUG_DISABLE
#endif /* ifndef NDEBUG */


typedef uint8_t SQUARE;
enum {
  A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63,
  A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
  A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
  A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
  A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
  A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
  A2 = 8,  B2 = 9,  C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
  A1 = 0,  B1 = 1,  C1 = 2,  D1 = 3,  E1 = 4,  F1 = 5,  G1 = 6,  H1 = 7,
  NO_SQUARE = 255
};

typedef uint8_t PIECE;
typedef uint8_t COLOUR;

#define NO_PIECE ((PIECE)0)
#define PAWN     ((PIECE)1)
#define KNIGHT   ((PIECE)2)
#define BISHOP   ((PIECE)3)
#define ROOK     ((PIECE)4)
#define QUEEN    ((PIECE)5)
#define KING     ((PIECE)6)

typedef enum {
  NO_PIECE_V = 0,
  PAWN_V     = 100,
  KNIGHT_V   = 300,
  BISHOP_V   = 340,
  ROOK_V     = 500,
  QUEEN_V    = 900,
  KING_V     = 10000 } PIECE_VALUE;

extern const int piece_values[];
extern const char * piece_names[];

#define WHITE ((COLOUR)0)
#define BLACK ((COLOUR)1)

extern const char * colour_names[];

typedef uint64_t BITBOARD;

#define BITBOARD_SCAN(bb) \
  for (BITBOARD __## bb ##__bb__ = bb; __## bb ##__bb__  > 0; __## bb ##__bb__ &= __## bb ##__bb__ - 1)

#define BITBOARD_SCAN_ITER(bb) (__builtin_ctzll(__## bb ##__bb__))

#define NO_CASTLE    ((CASTLE)0)
#define SHORT_CASTLE ((CASTLE)1)
#define LONG_CASTLE  ((CASTLE)2)

#define CALC_CASTLE(col, cas) (cas << (2 * col))
#define CASTLES_OF(col) (CALC_CASTLE((col), SHORT_CASTLE) | CALC_CASTLE((col), LONG_CASTLE))

#define WHITE_SHORT_CASTLE CALC_CASTLE(WHITE, SHORT_CASTLE)
#define WHITE_LONG_CASTLE  CALC_CASTLE(WHITE, LONG_CASTLE)
#define BLACK_SHORT_CASTLE CALC_CASTLE(BLACK, SHORT_CASTLE)
#define BLACK_LONG_CASTLE  CALC_CASTLE(BLACK, LONG_CASTLE)

#define ALL_CASTLES (WHITE_SHORT_CASTLE | WHITE_LONG_CASTLE | BLACK_SHORT_CASTLE | BLACK_LONG_CASTLE)

typedef uint8_t CASTLE;

/* max search depth supported */
#define MAX_PLIES        64
/* max half moves in a game */
#define MAX_GAME_PLIES 2048

#define ABS(a) (((a) < 0) ? -1 * (a) : (a))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define SIGNUM(a) ((a) ? ((a) > 0 ? 1 : -1) : 0)

#define LIGHT_SQUARES 0x55aa55aa55aa55aa
#define DARK_SQUARES  0xaa55aa55aa55aa55

#endif /* ifndef _CHESS_H_ */
