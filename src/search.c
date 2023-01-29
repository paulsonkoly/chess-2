#include "search.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <limits.h>
#include <assert.h>

#include "attacks.h"
#include "evaluate.h"
#include "move.h"
#include "movegen.h"
#include "moveexec.h"
#include "movelist.h"
#include "pv.h"
#include "chess.h"

static struct timespec start;
unsigned long long nodes;

#define STDIN 0
int check_for_input(void) {
  struct timespec timeout = { 0, 0 };
  fd_set fds;

  FD_ZERO(&fds);
  FD_SET(STDIN, &fds);

  pselect(1, &fds, NULL, NULL, & timeout, NULL);

  if (FD_ISSET(STDIN, &fds)) {
    char * line = NULL;
    size_t count;

    count = getline(&line, &count, stdin);

    if (strncmp(("stop"), line, strlen("stop")) == 0) {

      free(line);

      return 1;
    }

    free(line);
  }

  return 0;
}

unsigned long long int time_delta(void) {
  struct timespec end;
  unsigned long long int delta;

  if (clock_gettime(CLOCK_REALTIME, &end)) {
    printf("info clock_gettime failed\n");
  }

  delta = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;

  return delta;
}

extern int stopped;
unsigned long long movetime;

int repetition(const BOARD * board) {
  int ply;
  int cnt = 0;

  if (board->halfmovecnt > 2) {
    for (ply = board->halfmovecnt - 2; ply > 0; ply -= 2) {
      if ((board->history[ply].flags | board->history[ply+1].flags) & HIST_CANT_REPEAT) {
        return 0;
      }
      if (board->history[ply].hash == board->history[board->halfmovecnt].hash) {
        if (++cnt == 2) {
          return 1;
        }
      }
    }
  }

  return 0;
}

int quiesce(BOARD * board, int alpha, int beta) {
  MOVE * move;
  MOVEGEN_STATE mg_state = { MOVEGEN_START, MOVEGEN_FORCING, 0, 0 };
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

  while ((move = moves(board, 0, NULL, NULL, &mg_state))) {
    int score;

    execute_move(board, move);

    if (in_check(board, 1 - board->next)) {
      undo_move(board, move);
      continue;
    }

    score = -quiesce(board, -beta, -alpha);

    undo_move(board, move);

    if (score >= beta) {
      moves_done(&mg_state);
      return beta;
    }

    if (score > alpha)
      alpha = score;
  }

  return alpha;
}

/* * 100 */
static const int log[] = {
  -INT_MAX,
  0,   69,  109, 138, 160, 179, 194, 207, 219, 230,
  239, 248, 256, 263, 270, 277, 283, 289, 294, 299,
  304, 309, 313, 317, 321, 325, 329, 333, 336, 340,
  343, 346, 349, 352, 355, 358, 361, 363, 366, 368,
  371, 373, 376, 378, 380, 382, 385, 387, 389, 391,
  393, 395, 397, 398, 400, 402, 404, 406, 407, 409,
  411, 412, 414, 415, 417, 418, 420, 421, 423, 424,
  426, 427, 429, 430, 431, 433, 434, 435, 436, 438,
  439, 440, 441, 443, 444, 445, 446, 447, 448, 449,
  451, 452, 453, 454, 455, 456, 457, 458, 459, 460
};

static int lmr(int depth, int count) {
  int value = (log[depth] * log[count] / 19500);

#ifndef NDEBUG
  int s = sizeof(log) / sizeof(log[1]);

  assert(0 <= depth && depth < s && 0 <= count && count < s);
#endif

  return value > depth - 1 ? 0 : depth - 1 - value;
}

int negascout(BOARD* board,
    int ply,
    int depth,
    int reduced_depth,
    int alpha,
    int beta,
    const PV * opv,
    PV ** npv,
    KILLER * killer) {
  PV * lpv;
  int legal_found = 0;
  int score;
  int beta2;
  unsigned long long delta;
  int count;
  MOVE * move;
  MOVEGEN_STATE mg_state = { MOVEGEN_START, MOVEGEN_SORT, 0, 0 };

  assert(0 <= reduced_depth && reduced_depth <= depth);

  nodes++;

  if (stopped) {
    return -1;
  }
  else {
    if (check_for_input()) {
      stopped = 1;
      return -1;
    }

    if (movetime) {
      delta = time_delta();
      if (movetime < delta) {
        stopped = 1;
        return -1;
      }
    }
  }

  if (reduced_depth == 0) {
    return quiesce(board, alpha, beta);
  }

  if (repetition(board)) {
    return 0;
  }

  lpv = pv_init();

  beta2 = beta;

  count = 1;

  while ((move = moves(board, ply, opv, killer, &mg_state))) {

    execute_move(board, move);

    if (in_check(board, 1 - board->next)) {
      undo_move(board, move);
      continue;
    }

    pv_reset(lpv);

    score = -negascout(board, ply + 1, depth - 1, lmr(reduced_depth, count), -beta2, -alpha, opv, &lpv, killer);

    if (alpha < score && score < beta && legal_found) {
      score = -negascout(board, ply + 1, depth - 1, lmr(reduced_depth, count), -beta, -alpha, opv, &lpv, killer);
    }

    undo_move(board, move);

    if (alpha < score || (alpha == score && !legal_found)) {
      pv_insert(lpv, move, ply);
      pv_swap(&lpv, npv);
      alpha = score;
    }

    legal_found = 1;

    if (alpha >= beta) {
      save_killer(killer, ply, move);

      pv_destroy(lpv);

      moves_done(&mg_state);

      return alpha;
    }

    beta2 = alpha + 1;

    count++;
  }

  pv_destroy(lpv);

  if (!legal_found) {
    if (in_check(board, board->next))
      return -10000;
    else
      return 0;
  }

  return alpha;
}

#define MOVES_TO_GO 25

int iterative_deepening(BOARD * board, const SEARCH_LIMIT * search_limit) {
  PV * opv; /* old PV */
  PV * npv; /* next PV */
  KILLER killer;
  int max_depth = 0;
  int score = 0;
  const MOVE * bestmove = NULL;
  unsigned long long int delta;

  if (clock_gettime(CLOCK_REALTIME, &start)) {
    printf("info clock_gettime failed\n");
  }

  switch (search_limit->type) {
    case SL_INFINITE: max_depth = 1000; movetime = 0; break;
    case SL_DEPTH: max_depth = search_limit->data.depth; movetime = 0; break;
    case SL_MOVETIME: max_depth = 1000; movetime = search_limit->data.movetime; break;
    case SL_WBTIME: {
      unsigned long long time = 0;
      unsigned long long inc = 0;

      switch (board->next) {

        case WHITE:
          time = search_limit->data.wb_time.wtime;
          inc = search_limit->data.wb_time.winc;
          break;

        case BLACK:
          time = search_limit->data.wb_time.btime;
          inc = search_limit->data.wb_time.binc;
          break;
      }

      max_depth = MAX_PLIES; /* we would overrun our stores otherwise */
      movetime = (time + MOVES_TO_GO * inc) / MOVES_TO_GO;
      movetime = movetime > time / 2 ? time / 2 : movetime;

      break;
    }
  }

  if ((NULL == (opv = pv_init())) || (NULL == (npv = pv_init()))) {
    return 0;
  }

  for (int depth = 1; depth <= max_depth; ++depth) {

    delta = time_delta();

    /* assuming an exponential growth - if we used up 80% of the time, don't bother with the next ply */
    if (movetime && bestmove && delta * 10 > movetime * 8) {
      break;
    }

    pv_reset(npv);
    reset_killer(&killer);

    stopped = 0;
    nodes = 0;

    printf("info depth %d\n", depth);

    score = negascout(board, 0, depth, depth , -10000, 10000, opv, &npv, &killer);

    if (stopped) break;

    delta = time_delta();

    if (delta >= 1000) {
      printf("info nodes %llu nps %llu\n", nodes, nodes / (delta / 1000));
    }
    printf("info score cp %d depth %d time %llu pv ", score, depth, delta);

    for (int i = 0; i < pv_count(npv); ++i) {
      const MOVE * ptr = pv_get_move(npv, i);

      if (i == 0) {
        bestmove = ptr;
      }
      print_move(ptr);
      printf(" ");
    }
    printf("\n");

    fflush(stdout);

    if (score <= -10000 || score >= 10000) break;

    pv_swap(&opv, &npv);
  }

  if (bestmove) {
    printf("bestmove ");
    print_move(bestmove);
    printf("\n");
  }

  pv_destroy(opv);
  pv_destroy(npv);

  return score;
}

