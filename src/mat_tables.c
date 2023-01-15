#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mat_tables.h"

MAT_TABLE_ENTRY * mat_table = NULL;

#define MAX_PT 12

typedef enum {
  WP = 0,     /* white pawns */
  WN = 1,     /* white knights */
  WB_LSQ = 2, /* white bishops (light square) */
  WB_DSQ = 3, /* white bishops (dark square) */
  WR = 4,     /* white rooks */
  WQ = 5,     /* white queens */

  BP = 6,     /* black pawns */
  BN = 7,     /* black knights */
  BB_LSQ = 8, /* black bishops (light square) */
  BB_DSQ = 9, /* black bishops (dark square) */
  BR = 10,    /* black rooks */
  BQ = 11     /* black queens */
} PT_TYPES;

static const int max_counts[MAX_PT] = {
  /* 2 * (pawns, knights, lsq bishops, dsq bishops, rooks, queens) */
   8, 2, 1, 1, 2, 1,
   8, 2, 1, 1, 2, 1,
};

typedef struct {
  const char * str;
  MAT_TABLE_ENTRY entry;
} RULE;

/* when DRAWN rules match subsequent rules are not tried
 * matching rule values are summed up, and flags accumulated
 */
static const RULE rules[] = {
  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { "0  *  0      0      0  0  0  *  0      0      0  0", {     0,   DRAWN} }, /* N vs N/BLSQ/BDSQ */
  { "0  *  0      0      0  0  0  0  *      0      0  0", {     0,   DRAWN} },
  { "0  *  0      0      0  0  0  0  0      *      0  0", {     0,   DRAWN} },

  { "0  0  *      0      0  0  0  *  0      0      0  0", {     0,   DRAWN} }, /* BLSQ vs N/BLSQ/BDSQ*/
  { "0  0  *      0      0  0  0  0  *      0      0  0", {     0,   DRAWN} },
  { "0  0  *      0      0  0  0  0  0      *      0  0", {     0,   DRAWN} },

  { "0  0  0      *      0  0  0  *  0      0      0  0", {     0,   DRAWN} }, /* BDSQ vs N/BLSQ/BDSQ*/
  { "0  0  0      *      0  0  0  0  *      0      0  0", {     0,   DRAWN} },
  { "0  0  0      *      0  0  0  0  0      *      0  0", {     0,   DRAWN} },

  { "0  1  1      0      0  0  0  1  0      0      0  0", {     0,   DRAWN} }, /* B+N vs B/N/R */
  { "0  1  0      1      0  0  0  1  0      0      0  0", {     0,   DRAWN} },
  { "0  1  1      0      0  0  0  0  1      0      0  0", {     0,   DRAWN} },
  { "0  1  0      1      0  0  0  0  1      0      0  0", {     0,   DRAWN} },
  { "0  1  1      0      0  0  0  0  0      1      0  0", {     0,   DRAWN} },
  { "0  1  0      1      0  0  0  0  0      1      0  0", {     0,   DRAWN} },
  { "0  1  1      0      0  0  0  0  0      0      1  0", {     0,   DRAWN} },
  { "0  1  0      1      0  0  0  0  0      0      1  0", {     0,   DRAWN} },
  { "0  1  0      0      0  0  0  1  1      0      0  0", {     0,   DRAWN} },
  { "0  1  0      0      0  0  0  1  0      1      0  0", {     0,   DRAWN} },
  { "0  0  1      0      0  0  0  1  1      0      0  0", {     0,   DRAWN} },
  { "0  0  1      0      0  0  0  1  0      1      0  0", {     0,   DRAWN} },
  { "0  0  0      1      0  0  0  1  1      0      0  0", {     0,   DRAWN} },
  { "0  0  0      1      0  0  0  1  0      1      0  0", {     0,   DRAWN} },
  { "0  0  0      0      1  0  0  1  1      0      0  0", {     0,   DRAWN} },
  { "0  0  0      0      1  0  0  1  0      1      0  0", {     0,   DRAWN} },

  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { "0  0  0      0      1  0  0  0  1      0      0  0", {     0,   DRAWN} }, /* R vs B */
  { "0  0  0      0      1  0  0  0  0      1      0  0", {     0,   DRAWN} },
  { "0  0  1      0      0  0  0  0  0      0      1  0", {     0,   DRAWN} },
  { "0  0  0      1      0  0  0  0  0      0      1  0", {     0,   DRAWN} },

  { "0  0  0      0      1  0  0  1  0      0      0  0", {     0,   DRAWN} }, /* R vs N */
  { "0  1  0      0      0  0  0  0  0      0      1  0", {     0,   DRAWN} },

  { "0  0  1      0      1  0  0  0  0      0      1  0", {     0,   DRAWN} }, /* R+B vs R */
  { "0  0  0      1      1  0  0  0  0      0      1  0", {     0,   DRAWN} },
  { "0  0  0      0      1  0  0  0  1      0      1  0", {     0,   DRAWN} },
  { "0  0  0      0      1  0  0  0  0      1      1  0", {     0,   DRAWN} },

  { "0  1  0      0      1  0  0  0  0      0      1  0", {     0,   DRAWN} }, /* R+N vs R */
  { "0  0  0      0      1  0  0  1  0      0      1  0", {     0,   DRAWN} },

  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { "0  0  0      0      0  1  0  0  1      1      0  0", {     0,   DRAWN} }, /* Q vs B+B */
  { "0  0  1      1      0  0  0  0  0      0      0  1", {     0,   DRAWN} },

  { "0  0  0      0      0  1  0  2  0      0      0  0", {     0,   DRAWN} }, /* Q vs N+N */
  { "0  2  0      0      0  0  0  0  0      0      0  1", {     0,   DRAWN} },

  { "0  1  0      0      0  1  0  0  0      0      0  1", {     0,   DRAWN} }, /* Q+N vs Q */
  { "0  0  0      0      0  1  0  1  0      0      0  1", {     0,   DRAWN} },

  { "0  0  1      0      0  1  0  0  0      0      0  1", {     0,   DRAWN} }, /* Q+B vs Q */
  { "0  0  0      1      0  1  0  0  0      0      0  1", {     0,   DRAWN} },
  { "0  0  0      0      0  1  0  0  1      0      0  1", {     0,   DRAWN} },
  { "0  0  0      0      0  1  0  0  0      1      0  1", {     0,   DRAWN} },

  { ">0 0  0      0      0  0  0  1  0      0      0  0", {      KNIGHT_V,   0} }, /* piece can't win, against pawn, */
  { "0  1  0      0      0  0  >0 0  0      0      0  0", { -1 * KNIGHT_V,   0} },
  { ">0 0  0      0      0  0  0  0  1      0      0  0", {      BISHOP_V,   0} },
  { "0  0  1      0      0  0  >0 0  0      0      0  0", { -1 * BISHOP_V,   0} },
  { ">0 0  0      0      0  0  0  0  0      1      0  0", {      BISHOP_V,   0} },
  { "0  0  0      1      0  0  >0 0  0      0      0  0", { -1 * BISHOP_V,   0} },

  /* bishop and knight check mates */
  { "0  1  1      0      0  0  0  0  0      0      0  0", {     0,   W_CHECKMATING | BN_MATE_LSQ} },
  { "0  1  0      1      0  0  0  0  0      0      0  0", {     0,   W_CHECKMATING | BN_MATE_DSQ} },
  { "0  0  0      0      0  0  0  1  1      0      0  0", {     0,   B_CHECKMATING | BN_MATE_LSQ} },
  { "0  0  0      0      0  0  0  1  0      1      0  0", {     0,   B_CHECKMATING | BN_MATE_DSQ} },

  /* R/Q checkmating but we have to use the king */
  { "0  0  0      0      1  0  0  0  0      0      0  0", {     0,   W_CHECKMATING} },
  { "0  0  0      0      0  1  0  0  0      0      0  0", {     0,   W_CHECKMATING} },
  { "0  0  0      0      0  0  0  0  0      0      1  0", {     0,   B_CHECKMATING} },
  { "0  0  0      0      0  0  0  0  0      0      0  1", {     0,   B_CHECKMATING} },

  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { ">a 0  1      0      0  0  a  0  0      1      0  0", {   -80,   0} },    /* opposite colour bishop endgames */
  { "a  0  1      0      0  0  >a 0  0      1      0  0", {    80,   0} },    /* penalty 80 for winning side */
  { ">a 0  0      1      0  0  a  0  1      0      0  0", {   -80,   0} },
  { "a  0  0      1      0  0  >a 0  1      0      0  0", {    80,   0} },

  { "a  0  1      1      b  c  a  1  1      0      b  c", {    30,   0} },    /* bishop pair vs knight and bishop */
  { "a  0  1      1      b  c  a  1  0      1      b  c", {    30,   0} },    /* bonus: 40 */
  { "a  1  1      0      b  c  a  0  1      1      b  c", {   -30,   0} },
  { "a  1  0      1      b  c  a  0  1      1      b  c", {   -30,   0} },
  { "a  0  1      1      b  c  a  2  0      0      b  c", {    40,   0} },    /* bishop pair vs 2 knights */
  { "a  2  0      0      b  c  a  0  1      1      b  c", {   -40,   0} },

  /* discourage trading a minor piece for pawns */
  { "a >b  c      d      e  f  >a b  c      d      e  f", {    30,   0} },
  { "a  b >c      d      e  f  >a b  c      d      e  f", {    30,   0} },
  { "a  b  c      >d     e  f  >a b  c      d      e  f", {    30,   0} },
  { ">a b  c      d      e  f  a  >b c      d      e  f", {   -30,   0} },
  { ">a b  c      d      e  f  a  b  >c     d      e  f", {   -30,   0} },
  { ">a b  c      d      e  f  a  b  c      >d     e  f", {   -30,   0} },
  /* knight and pawns against 2 bishops */
  { ">a >b 0      0      e  f  a  b  1      1      e  f", {  -190,   0} },
  { "a  b  1      1      e  f  >a >b 0      0      e  f", {   190,   0} },

  /* discourage trading Q for R/(B/N)/P */
  { ">=a >b  c    d     >e  f  a  b  c      d      e >f", {   -60,   0} },
  { ">=a  b >c    d     >e  f  a  b  c      d      e >f", {   -60,   0} },
  { ">=a  b  c   >d     >e  f  a  b  c      d      e >f", {   -60,   0} },
  { "a  b  c      d      e >f >=a >b  c     d     >e  f", {    60,   0} },
  { "a  b  c      d      e >f >=a  b >c     d     >e  f", {    60,   0} },
  { "a  b  c      d      e >f >=a  b  c    >d     >e  f", {    60,   0} },

  { "*  *  *      *      *  *  *  *  *      *      *  *", {     0,   0} },    /* catch all */

  { NULL,                                                 {     0,   0} }
};

static int match(const char * token, int counts[]);

void initialize_mat_tables() {
  int counts[MAX_PT] = {
    0, 0, 0, 0, 0, 0,  /* white piece counts */
    0, 0, 0, 0, 0, 0 };/* black piece counts */

  MAT_TABLE_ENTRY * entry;

  if (NULL == (mat_table = calloc(9 * 9 * 3 * 3 * 2 * 2 * 2 * 2 * 3 * 3 * 2 *2, sizeof(MAT_TABLE_ENTRY)))) {
    abort();
  }

  while (1) {
    int i, j;
    int fin;
    int matched = 0;
    int value = 0;
    unsigned flags = 0;
    const RULE * rule = rules;

    while (rule->str) {
      if (match(rule->str, counts)) {
        matched = 1;

        value += rule->entry.value;
        flags |= rule->entry.flags;

        if (rule->entry.flags & DRAWN) {
          break;
        }
      }
      rule++;
    }

    if (! matched) {
      printf("The following material combination didn't match any rule:\n");

      printf("WP\t| WN\t| WBLSQ\t| WBDSQ\t| WR\t| WQ\t| BP\t| BN\t| BBLSQ\t| BBDSQ\t| BR\t| BQ\n");
      printf("%d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\n",
          counts[0], counts[1], counts[2], counts[3], counts[4],  counts[5],
          counts[6], counts[7], counts[8], counts[9], counts[10], counts[11]);

      rule -= 2; /* debug, point it to the rule that;s supposed to match and run in debugger */
      (volatile int)match(rule->str, counts);

      abort();
    }

    entry = & mat_table[
              counts[WP]                              +
              counts[WN]                          * 8 +
              counts[WB_LSQ]                  * 8 * 3 +
              counts[WB_DSQ]              * 8 * 3 * 2 +
              counts[WR]              * 8 * 3 * 2 * 2 +
              counts[WQ]          * 8 * 3 * 2 * 2 * 3 +
              counts[BP]      * 8 * 3 * 2 * 2 * 3 * 2 +
              counts[BN]      * 8 * 3 * 2 * 2 * 3 * 2                 * 8 +
              counts[BB_LSQ]  * 8 * 3 * 2 * 2 * 3 * 2             * 8 * 3 +
              counts[BB_DSQ]  * 8 * 3 * 2 * 2 * 3 * 2         * 8 * 3 * 2 +
              counts[BR]      * 8 * 3 * 2 * 2 * 3 * 2     * 8 * 3 * 2 * 2 +
              counts[BQ]      * 8 * 3 * 2 * 2 * 3 * 2 * 8 * 3 * 2 * 2 * 3];

    /* calculate endgame factor */
    /* total start material : 39 for each side - linear interpolate for each side and then average */
    int wmat = counts[WP] + 3 * counts[WN] + 3 * counts[WB_LSQ] + 3 * counts[WB_DSQ] + 5 * counts[WR] + 9 * counts[WQ];
    int bmat = counts[BP] + 3 * counts[BN] + 3 * counts[BB_LSQ] + 3 * counts[BB_DSQ] + 5 * counts[BR] + 9 * counts[BQ];
    int avg = (wmat + bmat) / 2;
    int endgame = avg / 10;

    assert(0 <= endgame && endgame < 4);
    flags |= (3 - endgame);

    entry->value = value;
    entry->flags = flags;

    /* next entry */
    fin = 1;
    for (i = 0; i < MAX_PT; ++i) {
      if (counts[i] < max_counts[i]) {
        counts[i]++;
        for (j = 0; j < i; ++j) {
          counts[j] = 0;
        }
        fin = 0;
        break;
      }
    }
    if (fin) {
      return;
    }
  }
}

int match(const char * str, int counts[]) {
  int i;
  int vars[12] = { 0 };
  const char * token = str;

  /* first pass, fill variables */
  for (i = 0; i < MAX_PT; ++i) {
    while (*token == ' ') token++;

    switch (*token) {
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        vars[*token - 'a'] = counts[i];
        token++;
        break;

      case '<': case '>':
        token++;
        if (*token == '=') {
          token++;
        }

      default:
        token++; /* operand */
    }
  }

  token = str;
  for (i = 0; i < MAX_PT; ++i) {
    int value;
    int op;

    while (*token == ' ') token++;

    switch (*token) {

      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
        if (*token - '0' == counts[i]) {
          token++;
        } else {
          return 0;
        }
        break;

      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        if (vars[*token - 'a'] == counts[i]) {
          token++;
        } else {
          return 0;
        }
        break;

      case '*':
        token++;
        break;

      case '>': case '<':
        op = *token++ == '>' ? 0 : 2;

        if (*token == '=') {
          token++;
          op++;
        }
        value = (*token < 'a' ? *token - '0' : vars[*token - 'a']);
        assert(0 <= value && value <= 8);

        switch (op) {
          case 0:
            if (value >= counts[i]) {
              return 0;
            }
            break;
          case 1:
            if (value > counts[i]) {
              return 0;
            }
            break;
          case 2:
            if (value <= counts[i]) {
              return 0;
            }
            break;
          case 3:
            if (value < counts[i]) {
              return 0;
            }
            break;
        }
        token++;
        break;

      default:;
    }
  }

  return 1;
}

const MAT_TABLE_ENTRY * get_mat_table_entry(const BOARD * board) {
  int counts[MAX_PT];

  counts[WP] = MIN(__builtin_popcountll(board->pawns & COLOUR_BB(board, WHITE)), max_counts[WP]);
  counts[WN] = MIN(__builtin_popcountll(board->knights & COLOUR_BB(board, WHITE)), max_counts[WN]);
  counts[WB_LSQ] = MIN(__builtin_popcountll(board->bishops & COLOUR_BB(board, WHITE) & LIGHT_SQUARES), max_counts[WB_LSQ]);
  counts[WB_DSQ] = MIN(__builtin_popcountll(board->bishops & COLOUR_BB(board, WHITE) & DARK_SQUARES), max_counts[WB_DSQ]);
  counts[WR] = MIN(__builtin_popcountll(board->rooks & COLOUR_BB(board, WHITE)), max_counts[WR]);
  counts[WQ] = MIN(__builtin_popcountll(board->queens & COLOUR_BB(board, WHITE)), max_counts[WQ]);

  counts[BP] = MIN(__builtin_popcountll(board->pawns & COLOUR_BB(board, BLACK)), max_counts[BP]);
  counts[BN] = MIN(__builtin_popcountll(board->knights & COLOUR_BB(board, BLACK)), max_counts[BN]);
  counts[BB_LSQ] = MIN(__builtin_popcountll(board->bishops & COLOUR_BB(board, BLACK) & LIGHT_SQUARES), max_counts[BB_LSQ]);
  counts[BB_DSQ] = MIN(__builtin_popcountll(board->bishops & COLOUR_BB(board, BLACK) & DARK_SQUARES), max_counts[BB_DSQ]);
  counts[BR] = MIN(__builtin_popcountll(board->rooks & COLOUR_BB(board, BLACK)), max_counts[BR]);
  counts[BQ] = MIN(__builtin_popcountll(board->queens & COLOUR_BB(board, BLACK)), max_counts[BQ]);

  return & mat_table[
      counts[WP]                              +
      counts[WN]                          * 8 +
      counts[WB_LSQ]                  * 8 * 3 +
      counts[WB_DSQ]              * 8 * 3 * 2 +
      counts[WR]              * 8 * 3 * 2 * 2 +
      counts[WQ]          * 8 * 3 * 2 * 2 * 3 +
      counts[BP]      * 8 * 3 * 2 * 2 * 3 * 2 +
      counts[BN]      * 8 * 3 * 2 * 2 * 3 * 2                 * 8 +
      counts[BB_LSQ]  * 8 * 3 * 2 * 2 * 3 * 2             * 8 * 3 +
      counts[BB_DSQ]  * 8 * 3 * 2 * 2 * 3 * 2         * 8 * 3 * 2 +
      counts[BR]      * 8 * 3 * 2 * 2 * 3 * 2     * 8 * 3 * 2 * 2 +
      counts[BQ]      * 8 * 3 * 2 * 2 * 3 * 2 * 8 * 3 * 2 * 2 * 3];
}

