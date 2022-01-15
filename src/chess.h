#ifndef _CHESS_H_
#define _CHESS_H_

#include <stdint.h>
#include <strings.h>

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

#endif /* ifndef _CHESS_H_ */
