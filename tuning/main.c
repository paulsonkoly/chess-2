/* Texel's tuning method */
/* https://www.chessprogramming.org/Texel%27s_Tuning_Method */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "board.h"
#include "quiesce.h"
#include "evaluate.h"
#include "mat_tables.h"
#include "tuned_values.h"

TUNED_VALUES tuned_values = {
  100, /* piece_v_pawn_v   */
  159, /* piece_v_knight_v */
  169, /* piece_v_bishop_v */
  367, /* piece_v_rook_v   */
  728, /* piece_v_queen_v  */
  0, /* pawn_ranks_0     */
  49, /* pawn_ranks_1     */
  25, /* pawn_ranks_2     */
  1, /* pawn_ranks_3     */
  11, /* pawn_ranks_4     */
  83, /* pawn_ranks_5     */
  186, /* pawn_ranks_6     */
  270, /* pawn_ranks_7     */
  -8, /* pawn_isolated    */
  -9, /* pawn_weak        */
  -2, /* king_shield_0    */
  -25, /* king_shield_1    */
  5, /* king_shield_2    */
  -9, /* king_shield_3    */
};

static float get_result(char * buffer) {
  int spaces = 0;

  /* go to 6th column */
  while (* buffer) {
    if (spaces == 5) break;
    if (*buffer == ' ') spaces++;
    buffer++;
  }

  if (strncmp(buffer, "1-0", 3) == 0) {
    return 1.0;
  } else if (strncmp(buffer, "0-1", 3) == 0) {
    return 0.0;
  } else if (strncmp(buffer, "1/2-1/2", 7) == 0) {
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
  printf("TUNED_VALUES tuned_values = {\n");
  printf("  %d, /* piece_v_pawn_v   */\n", tuned_values.piece_v_pawn_v);
  printf("  %d, /* piece_v_knight_v */\n", tuned_values.piece_v_knight_v);
  printf("  %d, /* piece_v_bishop_v */\n", tuned_values.piece_v_bishop_v);
  printf("  %d, /* piece_v_rook_v   */\n", tuned_values.piece_v_rook_v);
  printf("  %d, /* piece_v_queen_v  */\n", tuned_values.piece_v_queen_v);
  printf("  %d, /* pawn_ranks_0     */\n", tuned_values.pawn_ranks_0);
  printf("  %d, /* pawn_ranks_1     */\n", tuned_values.pawn_ranks_1);
  printf("  %d, /* pawn_ranks_2     */\n", tuned_values.pawn_ranks_2);
  printf("  %d, /* pawn_ranks_3     */\n", tuned_values.pawn_ranks_3);
  printf("  %d, /* pawn_ranks_4     */\n", tuned_values.pawn_ranks_4);
  printf("  %d, /* pawn_ranks_5     */\n", tuned_values.pawn_ranks_5);
  printf("  %d, /* pawn_ranks_6     */\n", tuned_values.pawn_ranks_6);
  printf("  %d, /* pawn_ranks_7     */\n", tuned_values.pawn_ranks_7);
  printf("  %d, /* pawn_isolated    */\n", tuned_values.pawn_isolated);
  printf("  %d, /* pawn_weak        */\n", tuned_values.pawn_weak);
  printf("  %d, /* king_shield_0    */\n", tuned_values.king_shield_0);
  printf("  %d, /* king_shield_1    */\n", tuned_values.king_shield_1);
  printf("  %d, /* king_shield_2    */\n", tuned_values.king_shield_2);
  printf("  %d, /* king_shield_3    */\n", tuned_values.king_shield_3);
  printf("};\n");
}

/* based on pseudo code from wiki */
void local_optimize(FILE * epd) {
  const int nParams = sizeof(tuned_values) / sizeof(int);
  float bestE = error(epd);

  int improved = 1;
  int count = 0;

  while (improved) {
    improved = 0;

    /* keep the pawn value at 100, to define the scale in cps */
    for (int pi = 1; pi < nParams; pi++) {

      ((int*) &tuned_values)[pi] += 1;

      double newE = error(epd);

      if (newE < bestE) {
        printf("error %f -> %f\n", bestE, newE);
        bestE = newE;
        improved = 1;
      } else {
        ((int*) &tuned_values)[pi] -= 2;
        newE = error(epd);
        if (newE < bestE) {
          printf("error %f -> %f\n", bestE, newE);
          bestE = newE;
          improved = 1;
        } else {
          ((int*) &tuned_values)[pi] += 1; /* reset to old value */
        }
      }
    }
    if (count++ % 1000) {
      print_values();
    }
  }
}

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
