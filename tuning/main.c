/* Texel's tuning method */
/* https://www.chessprogramming.org/Texel%27s_Tuning_Method */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "board.h"
#include "search.h"
#include "evaluate.h"
#include "mat_tables.h"
#include "tuned_values.h"

TUNED_VALUES tuned_values[4] = {
  /* (a) : passer rank 0
   * (b) : passer rank 1
   * (c) : passer rank 2
   * (d) : passer rank 3
   * (e) : passer rank 4
   * (f) : passer rank 5
   * (g) : passer rank 6
   * (h) : passer rank 7
   * (i) : isolated pawn
   * (j) : weak pawn
   * (k) : king shield
   */
  /*  P    K    B    R     Q  (a)  (b)  (c)  (d)  (e)  (f)  (g)  (h)  (i)  (j)  (k) */
  { 100, 159, 169, 367,  728,   0,  49,  25,   1,  11,  83, 186, 270,  -8,  -9,  -2 }, /* game phase (0) */
  { 100, 159, 169, 367,  728,   0,  49,  25,   1,  11,  83, 186, 270,  -8,  -9, -25 }, /* game phase (1) */
  { 100, 159, 169, 367,  728,   0,  49,  25,   1,  11,  83, 186, 270,  -8,  -9, -25 }, /* game phase (2) */
  { 100, 159, 169, 367,  728,   0,  49,  25,   1,  11,  83, 186, 270,  -8,  -9,  -9 }, /* game phase (3) */
};

static float get_result(char * buffer) {
  int spaces = 0;
  char * ptr = buffer;

  /* go to 6th column */
  while (* ptr) {
    if (spaces == 5) break;
    if (*ptr == ' ') spaces++;
    ptr++;
  }

  if (strncmp(ptr, "1-0", 3) == 0) {
    return 1.0;
  } else if (strncmp(ptr, "0-1", 3) == 0) {
    return 0.0;
  } else if (strncmp(ptr, "1/2-1/2", 7) == 0) {
    return 0.5;
  } else {
    printf("'%s' was invalid, no result string\n", buffer);
    abort();
  }

  return 0;
}

static COLOUR get_side_to_move(char * buffer) {
  int spaces = 0;

  /* go to 2nd column */
  while (* buffer) {
    if (spaces == 1) break;
    if (*buffer == ' ') spaces++;
    buffer++;
  }

  if (strncmp(buffer, "w", 1) == 0) {
    return WHITE;
  } else if (strncmp(buffer, "b", 1) == 0) {
    return BLACK;
  } else {
    printf("'%s' was invalid, no side to move\n", buffer);
    abort();
  }

  return 0;
}

static float sigmoid(int score) {
  return 1.0 / (1 + powf(10.0, (-1.0 * score) / 400.0));
}

static float error(FILE * epd) {
  rewind(epd);
  char buffer[1024];
  float error_sum = 0;
  int count = 0;

  while (NULL != fgets(buffer, 1024, epd)) {
    BOARD * board;
    int score;
    float result;
    COLOUR stm;
    float error;

    board = parse_fen(buffer);
    result = get_result(buffer);
    stm = get_side_to_move(buffer);

    score = quiesce(board, 0, -10000, 10000);
    if (stm == BLACK) {
      score *= -1;
    }

    error = powf(result - sigmoid(score), 2);
    error_sum += error;
    count++;

    free(board);

    /* printf("score: %d\t\tresult: %f error: %f\n", score, result, error); */
  }

  return error_sum / count;
}

static void print_values(void) {
  printf(
    "TUNED_VALUES tuned_values[4] = { {\n"
    " /* (a) : passer rank 0\n"
    "  * (b) : passer rank 1\n"
    "  * (c) : passer rank 2\n"
    "  * (d) : passer rank 3\n"
    "  * (e) : passer rank 4\n"
    "  * (f) : passer rank 5\n"
    "  * (g) : passer rank 6\n"
    "  * (h) : passer rank 7\n"
    "  * (i) : isolated pawn\n"
    "  * (j) : weak pawn\n"
    "  * (k) : king shield\n"
    "  */\n"
    " /*  P    K    B    R     Q  (a)  (b)  (c)  (d)  (e)  (f)  (g)  (h)  (i)  (j)  (k) */\n");
  for (int i = 0; i < 4; ++i) {
    printf("  {");
    for (int j = 0; j < sizeof(tuned_values[0]) / sizeof(int); ++j) {
      char buf[4];
      snprintf(buf, 4, "%d",  *(&tuned_values[i].piece_v_pawn_v + j));
      printf("%4s,", buf);
    }
    printf("  }, /* game phase (%i) */\n", i);
  }
}

/* based on pseudo code from wiki */
void local_optimize(FILE * epd) {
  const int nParams = sizeof(tuned_values[0]) / sizeof(int);
  float bestE = error(epd);

  int improved = 1;
  int count = 0;

  while (improved) {
    improved = 0;

    for (int i = 0; i < 4; ++i) {
      /* keep the pawn value at 100, to define the scale in cps */
      for (int pi = 1; pi < nParams; pi++) {

        ((int*) &tuned_values[i])[pi] += 1;

        double newE = error(epd);

        if (newE < bestE) {
          printf("error %f -> %f\n", bestE, newE);
          bestE = newE;
          improved = 1;
        } else {
          ((int*) &tuned_values[i])[pi] -= 2;
          newE = error(epd);
          if (newE < bestE) {
            printf("error %f -> %f\n", bestE, newE);
            bestE = newE;
            improved = 1;
          } else {
            ((int*) &tuned_values[i])[pi] += 1; /* reset to old value */
          }
        }
      }
    }
    if (count++ % 1000) {
      print_values();
    }
  }
}

int stopped = 0;
unsigned long long nodes;

int main(int argc, const char * argv[])
{
  FILE * epd;

  initialize_mat_tables();

  if (argc != 2 || NULL == (epd = fopen(argv[1], "r"))) {
    printf("Usage: %s epd_file\n", argv[0]);
    return -1;
  }

  local_optimize(epd);

  printf("new values:\n\n");
  print_values();

  fclose(epd);
  free(mat_table);

  return 0;
}
