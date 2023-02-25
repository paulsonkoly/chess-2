#include <stdlib.h>

#include "quiesce.h"
#include "move.h"
#include "attacks.h"
#include "evaluate.h"
#include "movegen.h"
#include "moveexec.h"

extern unsigned long long nodes;

int quiesce(BOARD * board, int ply, int alpha, int beta) {
  MOVE * move;
  int first = 1;
  int stand_pat;

  nodes++;

  if (repetition(board)) return 0;
  if (checkmate(board))  return -10000;
  if (stalemate(board))  return 0;

  stand_pat = evaluate(board);

  if (stand_pat >= beta) {
    return beta;
  }

  if (alpha < stand_pat)
    alpha = stand_pat;

  while ((move = moves(board, ply, NULL, NULL, MOVEGEN_FORCING_ONLY, first))) {
    int score;
    first = 0;

    execute_move(board, move);

    if (in_check(board, 1 - board->next)) {
      undo_move(board, move);
      continue;
    }

    score = -quiesce(board, ply + 1, -beta, -alpha);

    undo_move(board, move);

    if (score >= beta) {
      moves_done(ply);
      return beta;
    }

    if (score > alpha)
      alpha = score;
  }

  return alpha;
}
