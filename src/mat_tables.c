#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct __MAT_TABLE_ENTRY__ {
/* #define ENDGAME_FACTOR_MASK 0x00000003 */
#define DRAWN               0x00000004 /* insufficient material */
  int value;
  uint32_t flags;
} MAT_TABLE_ENTRY;

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

typedef struct {
  const char * str;
  MAT_TABLE_ENTRY entry;
} RULE;

/* first matching rule applies
 * last rule catches all, if that matches juts use normal evaluation
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

  { "*  *  *      *      >0 *  0  0  0      0      0  0", {  8000,   0} },     /* Q/R vs - */
  { "*  *  *      *      * >0  0  0  0      0      0  0", {  8000,   0} },
  { "0  0  0      0      0  0  *  *  *      *      >0 *", { -8000,   0} },
  { "0  0  0      0      0  0  *  *  *      *      * >0", { -8000,   0} },

  { "*  *  >0     >0     *  *  0  0  0      0      0  0", {  5000,   0} },     /* B+B vs - */
  { "0  0  0      0      0  0  *  *  >0     >0     *  *", { -5000,   0} },

  { "*  >0 >0     *      *  *  0  0  0      0      0  0", {  2000,   0} },     /* B+N vs - */
  { "0  0  0      0      0  0  *  >0 >0     *      *  *", { -2000,   0} },
  { "*  >0 *      >0     *  *  0  0  0      0      0  0", {  2000,   0} },
  { "0  0  0      0      0  0  *  >0 *      >0     *  *", { -2000,   0} },

  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { "*  *  *      *      *  >0 0  0  0      0      1  0", {  2000,   0} },     /* Q vs R/B/N */
  { "*  *  *      *      *  >0 0  0  0      1      0  0", {  3000,   0} },
  { "*  *  *      *      *  >0 0  0  1      0      0  0", {  3000,   0} },
  { "*  *  *      *      *  >0 0  1  0      0      0  0", {  3000,   0} },
  { "0  0  0      0      1  0  *  *  *      *      * >0", { -2000,   0} },
  { "0  0  0      1      0  0  *  *  *      *      * >0", { -3000,   0} },
  { "0  0  1      0      0  0  *  *  *      *      * >0", { -3000,   0} },
  { "0  1  0      0      0  0  *  *  *      *      * >0", { -3000,   0} },

  { "*  *  *      *      *  1  0  1  1      0      0  0", {  1000,   0} },     /* Q vs B+N */
  { "*  *  *      *      *  1  0  1  0      1      0  0", {  1000,   0} },
  { "0  1  1      0      0  0  *  *  *      *      *  1", {  1000,   0} },
  { "0  1  0      1      0  0  *  *  *      *      *  1", {  1000,   0} },

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

  { ">0 *  *      *      *  *  *  *  *      *      *  *", {     0,   0} },    /* CATCH ALL */
  { "*  *  *      *      *  *  >0 *  *      *      *  *", {     0,   0} },    /* CATCH ALL */

  { NULL,                                                 {     0,   0} }
};

static int match(const char * token, int counts[]);

void initialize_mat_tables() {
  int counts[MAX_PT] = {
    0, 0, 0, 0, 0, 0,  /* white piece counts */
    0, 0, 0, 0, 0, 0 };/* black piece counts */

  static const int max_counts[MAX_PT] = {
    /* 2 * (pawns, knights, lsq bishops, dsq bishops, rooks, queens) */
     8, 2, 1, 1, 2, 1,
     8, 2, 1, 1, 2, 1,
  };

  while (1) {
    int i, j;
    int fin;
    /* int value = 0; */
    /* unsigned flags = 0; */
    const RULE * rule = rules;

    while (rule->str) {
      if (match(rule->str, counts)) {

        break;
      }
      rule++;
    }
    if (NULL == rule->str) {
      printf("The following material combination didn't match any rule:\n");

      printf("WP\t| WN\t| WBLSQ\t| WBDSQ\t| WR\t| WQ\t| BP\t| BN\t| BBLSQ\t| BBDSQ\t| BR\t| BQ\n");
      printf("%d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\t| %d\n",
          counts[0], counts[1], counts[2], counts[3], counts[4],  counts[5],
          counts[6], counts[7], counts[8], counts[9], counts[10], counts[11]);

      abort();
    }

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

int match(const char * token, int counts[]) {
  int i;

  for (i = 0; i < MAX_PT; ++i) {

    while (*token == ' ') token++;

    switch (*token) {

      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
        if (*token - '0' == counts[i]) {
          token++;
        } else {
          return 0;
        }
        break;

      case '*':
        token++;
        break;

      case '>':
        token++;
        if (*token - '0' < counts[i]) {
          token++;
        } else {
          return 0;
        }

      default:;
    }
  }

  return 1;
}

