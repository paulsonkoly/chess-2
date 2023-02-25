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
  100,  /* piece_v_pawn_v   */
  300,  /* piece_v_knight_v */
  300,  /* piece_v_bishop_v */
  500,  /* piece_v_rook_v   */
  900,  /* piece_v_queen_v  */
  0,    /* pawn_ranks_0     */
  30,   /* pawn_ranks_1     */
  35,   /* pawn_ranks_2     */
  45,   /* pawn_ranks_3     */
  65,   /* pawn_ranks_4     */
  105,  /* pawn_ranks_5     */
  185,  /* pawn_ranks_6     */
  270,  /* pawn_ranks_7     */
  -20,  /* pawn_isolated    */
  -10,  /* pawn_weak        */
  -40,  /* king_shield_0    */
  -20,  /* king_shield_1    */
  0,    /* king_shield_2    */
  0,    /* king_shield_3    */
};

static float get_result(char * buffer) {
  int spaces = 0;

  /* go to 5th column */
  while (* buffer) {
    if (spaces == 4) break;
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

  /* go to 5th column */
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

    printf("score: %d\t\tresult: %f error: %f\n", score, result, error);
  }

  return error_sum / count;
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

  printf("overall average error: %f\n", error(epd));

  fclose(epd);
  free(mat_table);

  return 0;
}
