#include <stdio.h>
#include <stdlib.h>

#include "board.h"
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

int main()
{
  BOARD * board;
  int score;

  initialize_mat_tables();

  printf("tuning...\n");

  board = parse_fen("r6r/ppp1kppp/3p4/Q2Bn1q1/7N/2P4b/P4PPP/RN3RK1 b - - 0 15");
  score = evaluate(board);

  printf("score: %d\n", score);

  free(mat_table);

  return 0;
}
