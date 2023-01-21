#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mat_tables.h"

MAT_TABLE_ENTRY * mat_table = NULL;

#define MAT_TABLE_SIZE (9 * 9 * 3 * 3 * 2 * 2 * 2 * 2 * 3 * 3 * 2 * 2)
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

/* piece combinations match mini language
 * (c) Paul Sonkoly 2023
 *
 * Rules are tried in turn on a piece combination and when DRAWN rules match
 * subsequent rules are not tried. Matching rule values are summed up, and
 * flags accumulated.
 *
 * Language has 3 types of statements:
 *  - matcher rule
 *  - constraint group start
 *  - constraint group end
 *
 * Constraint group start
 * ----------------------
 * Use CONSTRAINT   in flags. rule->str is constraint string expressing
 * arithmetic, relational and logic expressions between variables and
 * constants.
 * Variables are filled from the matcher rule within the constraint group.
 *
 * Constraint group end
 * --------------------
 * Use with NULL rule->str. Clears active constraints.
 *
 * Matcher rule
 * ------------
 * Fills variables with value according to piece combination. Multiple
 * appearance of the same variable asserts matching values. Constants assert matching value.
 * Relational constraint asserts value fullfilling constraint.
 * '*' allows any value.
 */
static const RULE rules[] = {
  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { "a+b+c<=1 & d+e+f<=1                               ", {     0,   CONSTRAINT   } },
  { "0  a  b      c      0  0  0  d  e      f      0  0", {     0,   DRAWN} }, /* N/B vs N/B */
  { NULL,                                                 {     0,   CONSTRAINT   } },

  { "a+b=1 & c+d+e+f=1                                 ", {     0,   CONSTRAINT   } },
  { "0  1  a      b      0  0  0  c  d      e      f  0", {     0,   DRAWN} },  /* B+N vs B/N/R */
  { "0  c  d      e      f  0  0  1  a      b      0  0", {     0,   DRAWN} },
  { NULL,                                                 {     0,   CONSTRAINT   } },

  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { "a+b+c=1                                           ", {     0,   CONSTRAINT   } },
  { "0  0  0      0      1  0  0  a  b      c      0  0", {     0,   DRAWN} }, /* R vs B/N */
  { "0  a  b      c      0  0  0  0  0      0      1  0", {     0,   DRAWN} },
  { NULL,                                                 {     0,   CONSTRAINT   } },

  { "a+b+c=1                                           ", {     0,   CONSTRAINT   } },
  { "0  a  b      c      1  0  0  0  0      0      1  0", {     0,   DRAWN} }, /* R+B/N vs R */
  { "0  0  0      0      1  0  0  a  b      c      1  0", {     0,   DRAWN} },
  { NULL,                                                 {     0,   CONSTRAINT   } },

  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { "0  0  0      0      0  1  0  0  1      1      0  0", {     0,   DRAWN} }, /* Q vs B+B */
  { "0  0  1      1      0  0  0  0  0      0      0  1", {     0,   DRAWN} },

  { "0  0  0      0      0  1  0  2  0      0      0  0", {     0,   DRAWN} }, /* Q vs N+N */
  { "0  2  0      0      0  0  0  0  0      0      0  1", {     0,   DRAWN} },

  { "a+b+c=1                                           ", {     0,   CONSTRAINT   } },
  { "0  a  b      c      0  1  0  0  0      0      0  1", {     0,   DRAWN} }, /* Q+B vs Q */
  { "0  0  0      0      0  1  0  a  b      c      0  1", {     0,   DRAWN} },
  { NULL,                                                 {     0,   CONSTRAINT   } },

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
  { "a+b=1                                             ", {     0,   CONSTRAINT   } },
  { "0  0  0      0      a  b  0  0  0      0      0  0", {     0,   W_CHECKMATING} },
  { "0  0  0      0      0  0  0  0  0      0      a  b", {     0,   B_CHECKMATING} },
  { NULL,                                                 {     0,   CONSTRAINT   } },

  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */
  { "a+1=b                                             ", {     0,   CONSTRAINT   } },
  { "a  0  1      0      0  0  b  0  0      1      0  0", {    PAWN_V, 0} },    /* opposite colour bishop endgames */
  { "b  0  1      0      0  0  a  0  0      1      0  0", { -1*PAWN_V, 0} },    /* 1 pawn diff: take away the pawn */
  { "a+2=b                                             ", {     0,   CONSTRAINT   } },
  { "a  0  1      0      0  0  b  0  0      1      0  0", {    80,   0} },      /* 2 pawn diff: take away less than a pawn */
  { "b  0  1      0      0  0  a  0  0      1      0  0", {   -80,   0} },      /* 3 pawn diff: should be winning? */
  { NULL,                                                 {     0,   CONSTRAINT   } },

  { "a  0  1      1      b  c  a  1  1      0      b  c", {    30,   0} },    /* bishop pair vs knight and bishop */
  { "a  0  1      1      b  c  a  1  0      1      b  c", {    30,   0} },    /* bonus: 40 */
  { "a  1  1      0      b  c  a  0  1      1      b  c", {   -30,   0} },
  { "a  1  0      1      b  c  a  0  1      1      b  c", {   -30,   0} },
  { "a  0  1      1      b  c  a  2  0      0      b  c", {    40,   0} },    /* bishop pair vs 2 knights */
  { "a  2  0      0      b  c  a  0  1      1      b  c", {   -40,   0} },

  /* discourage trading a minor piece for pawns, assuming we can still win - not only the minor piece left */
  { "a+b+c+d+e+f>1                                     ", {     0,   CONSTRAINT   } },
  { "a  b  c      d      e  f  >a <b c      d      e  f", {   110,   0} },
  { "a  b  c      d      e  f  >a b  <c     d      e  f", {   110,   0} },
  { "a  b  c      d      e  f  >a b  c      <d     e  f", {   110,   0} },
  { ">a <b c      d      e  f  a  b  c      d      e  f", {  -110,   0} },
  { ">a b  <c     d      e  f  a  b  c      d      e  f", {  -110,   0} },
  { ">a b  c      <d     e  f  a  b  c      d      e  f", {  -110,   0} },
  { NULL,                                                 {     0,   CONSTRAINT   } },
  /* knight and pawns against 2 bishops */
  { ">a >b 0      0      e  f  a  b  1      1      e  f", {  -190,   0} },
  { "a  b  1      1      e  f  >a >b 0      0      e  f", {   190,   0} },

  /* discourage trading Q for R+(B/N)+P */
  { ">=a >b  c    d     >e  f  a  b  c      d      e >f", {   -60,   0} },
  { ">=a  b >c    d     >e  f  a  b  c      d      e >f", {   -60,   0} },
  { ">=a  b  c   >d     >e  f  a  b  c      d      e >f", {   -60,   0} },
  { "a  b  c      d      e >f >=a >b  c     d     >e  f", {    60,   0} },
  { "a  b  c      d      e >f >=a  b >c     d     >e  f", {    60,   0} },
  { "a  b  c      d      e >f >=a  b  c    >d     >e  f", {    60,   0} },

  /* WP WN WB_LSQ WB_DSQ WR WQ BP BN BB_LSQ BB_DSQ BR BQ        V    F */

  /* pawn advantage greater with less pieces */
  { "b+c+d+e+f=0                                       ", {     0,   CONSTRAINT   } },
  { "a  b  c      d      e  f >a  b  c      d      e  f", {   -40,   0} },
  { "b+c+d+e+f=1                                       ", {     0,   CONSTRAINT   } },
  { "a  b  c      d      e  f >a  b  c      d      e  f", {   -30,   0} },
  { "b+c+d+e+f=2                                       ", {     0,   CONSTRAINT   } },
  { "a  b  c      d      e  f >a  b  c      d      e  f", {   -25,   0} },
  { "b+c+d+e+f=3                                       ", {     0,   CONSTRAINT   } },
  { "a  b  c      d      e  f >a  b  c      d      e  f", {   -20,   0} },
  { "b+c+d+e+f=4                                       ", {     0,   CONSTRAINT   } },
  { "a  b  c      d      e  f >a  b  c      d      e  f", {   -15,   0} },
  { "b+c+d+e+f=5                                       ", {     0,   CONSTRAINT   } },
  { "a  b  c      d      e  f >a  b  c      d      e  f", {   -10,   0} },
  { "b+c+d+e+f=6                                       ", {     0,   CONSTRAINT   } },
  { "a  b  c      d      e  f >a  b  c      d      e  f", {    -5,   0} },

  { "b+c+d+e+f=0                                       ", {     0,   CONSTRAINT   } },
  { ">a b  c      d      e  f a   b  c      d      e  f", {    40,   0} },
  { "b+c+d+e+f=1                                       ", {     0,   CONSTRAINT   } },
  { ">a b  c      d      e  f a   b  c      d      e  f", {    30,   0} },
  { "b+c+d+e+f=2                                       ", {     0,   CONSTRAINT   } },
  { ">a b  c      d      e  f a   b  c      d      e  f", {    25,   0} },
  { "b+c+d+e+f=3                                       ", {     0,   CONSTRAINT   } },
  { ">a b  c      d      e  f a   b  c      d      e  f", {    20,   0} },
  { "b+c+d+e+f=4                                       ", {     0,   CONSTRAINT   } },
  { ">a b  c      d      e  f a   b  c      d      e  f", {    15,   0} },
  { "b+c+d+e+f=5                                       ", {     0,   CONSTRAINT   } },
  { ">a b  c      d      e  f a   b  c      d      e  f", {    10,   0} },
  { "b+c+d+e+f=6                                       ", {     0,   CONSTRAINT   } },
  { ">a b  c      d      e  f a   b  c      d      e  f", {     5,   0} },
  { NULL,                                                 {     0,   CONSTRAINT   } },

  { NULL,                                                 {     0,   0} }
};

static void fill_vars(const char * token, const int counts[], int vars[]);
static int match(const char * token, const int counts[], const int vars[]);
static int match_constraints(const char * constraints, const int vars[]);
static int endgame_factor(int counts[]);
static int mat_table_idx(int counts[]);

void initialize_mat_tables() {
  int counts[MAX_PT] = {
    0, 0, 0, 0, 0, 0,  /* white piece counts */
    0, 0, 0, 0, 0, 0 };/* black piece counts */

  MAT_TABLE_ENTRY * entry;

#if DEBUG
  int combo_matched = 0;
  int combo_unmatched = 0;
#endif

  if (NULL == (mat_table = calloc(MAT_TABLE_SIZE, sizeof(MAT_TABLE_ENTRY)))) {
    abort();
  }

  while (1) {
    int i, j;
    int fin;
    int value = 0;
    unsigned flags = 0;
    const RULE * rule = rules;
    const char * constraints = NULL;
#if DEBUG
    int matched = 0;
#endif

    while (rule->str || rule->entry.flags & CONSTRAINT) {
      int vars[12] = { 0 };

      if (rule->entry.flags & CONSTRAINT) {
        constraints = rule->str;
      } else {
        fill_vars(rule->str, counts, vars);

        if (match(rule->str, counts, vars) && match_constraints(constraints, vars)) {
#if DEBUG
          matched = 1;
#endif

          value += rule->entry.value;
          flags |= rule->entry.flags;

          if (rule->entry.flags & DRAWN) {
            break;
          }
        }
      }

      rule++;
    }
#if DEBUG
    if (matched) {
      combo_matched++;
    } else {
      combo_unmatched++;
    }
#endif

    flags |= endgame_factor(counts);

    entry = & mat_table[mat_table_idx(counts)];

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
#if DEBUG
      printf("mat tables %d matched by rule %d unmatched %d total\n",
          combo_matched, combo_unmatched, combo_matched + combo_unmatched );
#endif
      return;
    }
  }
}

void fill_vars(const char * str, const int counts[], int vars[]) {
  int i;
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
}

int match(const char * str, const int counts[], const int vars[]) {
  int i;
  const char * token = str;

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

#define OP_PLUS   0x01000000
#define OP_MINUS  0x02000000
#define OP_EQ     0x04000000
#define OP_LT     0x08000000
#define OP_GT     0x10000000
#define OP_LTE    0x20000000
#define OP_GTE    0x40000000
#define OP_AND    0x80000000
#define OP        0xff000000

static int precedence(int op) {
  switch (op) {
    case OP_PLUS: case OP_MINUS: return 2;
    case OP_EQ: case OP_LT: case OP_GT: case OP_LTE: case OP_GTE: return 1;
    case OP_AND: return 0;
  }

  assert(0);
  return 0;
}

#define STACK_SIZE 16

int match_constraints(const char * constraints, const int vars[]) {
  const char * token = constraints;
  int stack[STACK_SIZE];
  int stack_ptr = 0;
  int rev_polish[STACK_SIZE];
  int rev_polish_ptr = 0;
  int i;

  if (constraints == NULL) {
    return 1;
  }

  while (*token) {
    int itoken = 0;

    /* tokenize */
    switch (*token) {
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': itoken = vars[*token - 'a']; break;

      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
         itoken = *token - '0'; break;

      case '+': itoken = OP_PLUS; break;

      case '-': itoken = OP_MINUS; break;

      case '=': itoken = OP_EQ; break;

      case '&': itoken = OP_AND; break;

      case '<':
        if (*(token + 1) == '=') {
          token++;
          itoken = OP_LTE;
        } else itoken = OP_LT;
        break;

      case '>':
        if (*(token + 1) == '=') {
          token++;
          itoken = OP_GTE;
        } else itoken = OP_GT;
        break;

      default: token++; continue;
    }
    token++;

    /* shunting yard */
    if ((itoken & OP) == 0) { /* operand */
      assert(rev_polish_ptr < STACK_SIZE);
      rev_polish[rev_polish_ptr++] = itoken;
    } else { /* operator */
      while (stack_ptr > 0 && precedence(stack[stack_ptr - 1]) >= precedence(itoken)) {
        assert(rev_polish_ptr < STACK_SIZE);
        rev_polish[rev_polish_ptr++] = stack[--stack_ptr];
      }

      assert(stack_ptr < STACK_SIZE);
      stack[stack_ptr++] = itoken;
    }
  }
  while (stack_ptr > 0) {
    assert(rev_polish_ptr < STACK_SIZE);
    rev_polish[rev_polish_ptr++] = stack[--stack_ptr];
  }

  /* reverse polish evaluate */
  for (i = 0; i < rev_polish_ptr; ++i) {
    int itoken = rev_polish[i];

    if ((itoken & OP) == 0) { /* operand */
      stack[stack_ptr++] = itoken;
    } else { /* operator */
      assert(stack_ptr > 1);
      int rvalue = stack[--stack_ptr];
      int lvalue = stack[--stack_ptr];

      switch (itoken) {
        case OP_PLUS:  stack[stack_ptr++] = lvalue + rvalue; break;
        case OP_MINUS: stack[stack_ptr++] = lvalue - rvalue; break;
        case OP_EQ:    stack[stack_ptr++] = lvalue == rvalue; break;
        case OP_LT:    stack[stack_ptr++] = lvalue < rvalue; break;
        case OP_GT:    stack[stack_ptr++] = lvalue > rvalue; break;
        case OP_LTE:   stack[stack_ptr++] = lvalue <= rvalue; break;
        case OP_GTE:   stack[stack_ptr++] = lvalue >= rvalue; break;
        case OP_AND:   stack[stack_ptr++] = lvalue && rvalue; break;
      }
    }
  }
  assert(stack_ptr == 1);
  return stack[--stack_ptr];
}

int endgame_factor(int counts[]) {
    /* calculate endgame factor */
    /* total start material : 39 for each side - linear interpolate for each side and then average */
    int wmat = counts[WP] + 3 * counts[WN] + 3 * counts[WB_LSQ] + 3 * counts[WB_DSQ] + 5 * counts[WR] + 9 * counts[WQ];
    int bmat = counts[BP] + 3 * counts[BN] + 3 * counts[BB_LSQ] + 3 * counts[BB_DSQ] + 5 * counts[BR] + 9 * counts[BQ];
    int avg = (wmat + bmat) / 2;
    int endgame = avg / 10;

    assert(0 <= endgame && endgame < 4);

    return 3 - endgame;
}

static int mat_table_idx(int counts[]) {
  int result =
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
    counts[BQ]      * 8 * 3 * 2 * 2 * 3 * 2 * 8 * 3 * 2 * 2 * 3;

  assert(result < MAT_TABLE_SIZE);

  return result;
}

static inline void get_counts(const BOARD * board, int counts[]) {
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
}

const MAT_TABLE_ENTRY * get_mat_table_entry(const BOARD * board) {
  int counts[MAX_PT];

  get_counts(board, counts);

  return & mat_table[mat_table_idx(counts)];
}

#if DEBUG
void mat_table_debug(const BOARD * board) {
  int counts[MAX_PT];
  const RULE * rule = rules;
  const char * constraints = NULL;

  get_counts(board, counts);


  while (rule->str || rule->entry.flags & CONSTRAINT) {
    int vars[12] = { 0 };

    if (rule->entry.flags & CONSTRAINT) {
      constraints = rule->str;
      printf("constraints %s\n", constraints);
    } else {
      fill_vars(rule->str, counts, vars);

      printf("%s\n", rule->str);
      printf("a: %d b: %d c: %d d: %d e: %d f: %d\n", vars[0], vars[1], vars[2], vars[3], vars[4], vars[5]);
      printf("matched: %d constrains matched: %d\n",
          match(rule->str, counts, vars),
          match_constraints(constraints, vars));

      if (match(rule->str, counts, vars) && match_constraints(constraints, vars)) {
        printf("MATCHED\n");

        if (rule->entry.flags & DRAWN) {
          printf("DRAWN\n");
          return;
        }
      }
    }
    printf("\n");

    rule++;
  }
}
#define MAT_TABLE_DEBUG(board) mat_table_debug(board)
#else
#define MAT_TABLE_DEBUG(board)
#endif

