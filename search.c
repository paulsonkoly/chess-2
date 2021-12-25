#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "search.h"
#include "movegen.h"
#include "moveexec.h"

static struct timespec start;

#define STDIN 0
int check_for_input() {
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

unsigned long long int time_delta() {
  struct timespec end;
  unsigned long long int delta;

  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end)) {
    printf("info clock_gettime failed\n");
  }

  delta = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;

  return delta;
}

extern int stopped;
extern unsigned long long movetime;

int negascout(const BOARD* board, int depth, int alpha, int beta, int colour, MOVE * pv, MOVE * npv, KILLER * killer) {
  MOVE moves[100];
  MOVE lpv[30];
  MOVE * moveptr;
  BOARD copy;
  int legal_found = 0;
  int score;
  unsigned long long delta;

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

  if (depth < 1) {
    return colour * evaluate(board);
  }

  moveptr = add_moves(board, moves, depth <= 0);

  if (depth > 1) {
    /* find pv move among possible moves and bring forward */
    for (MOVE * ptr = moves; ptr != moveptr; ptr++) {
      if (ptr->from == pv->from && ptr->to == pv->to) {
        MOVE tmp;

        tmp = *ptr;
        *ptr = *moves;
        *moves = tmp;

        break;
      }
    }
  }

  apply_killers(killer, moves, moveptr, depth);

  for (MOVE * ptr = moves; ptr != moveptr; ptr++) {
    copy = *board;

    execute_move(&copy, ptr);

    if (in_check(&copy, 1 - copy.next)) {
      continue;
    }

    if (! legal_found) {
      score = -1 * negascout(&copy, depth - 1, -1 * beta, -1 * alpha, -1 * colour, pv + 1, lpv + 1, killer);
      legal_found = 1;
    }
    else {
      /* null window */
      score = -1 * negascout(&copy, depth - 1, -1 * alpha - 1, -1 * alpha, -1 * colour, pv + 1, lpv + 1, killer);
      if (alpha < score && score < beta) {
        score = -1 * negascout(&copy, depth - 1, -1 * beta, -1 * score, -1 * colour, pv + 1, lpv + 1, killer);
      }
    }
    if (alpha < score) {
      if (depth > 1) {
        memcpy(npv, lpv, sizeof(MOVE) * depth - 1);
      }
      *npv = *ptr;
      alpha = score;
    }

    if (alpha >= beta) {
      save_killer(killer, depth, ptr);
      break;
    }
  }

  if (!legal_found) {
    if (in_check(board, board->next))
      return board->next ? -1000 : 1000;
    else
      return 0;
  }

  return alpha;
}

int iterative_deepening(const BOARD * board, int max_depth) {
  MOVE pvm[2][80];
  KILLER killer;
  int score = 0;
  MOVE * bestmove = NULL;
  unsigned long long int delta;

  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start)) {
    printf("info clock_gettime failed\n");
  }

  for (int depth = 1; depth <= max_depth; ++depth) {
    int pvm_bank = depth & 1;

    stopped = 0;

    reset_killer(&killer);

    printf("info depth %d\n", depth);

    score = negascout(board, depth , -1000, 1000, board->next ? -1 : 1, pvm[1 - pvm_bank], pvm[pvm_bank], &killer);

    if (stopped) break;

    delta = time_delta();

    printf("info score cp %d depth %d time %llu pv ", 10 * score, depth, delta);

    for (int i = 0; i < depth; ++i) {
      print_move(&pvm[pvm_bank][i]);
      printf(" ");
    }
    printf("\n");

    fflush(stdout);

    bestmove = pvm[pvm_bank];

    if (score <= -1000 || score >= 1000) break;
  }

  if (bestmove) {
    printf("bestmove ");
    print_move(bestmove);
    printf("\n");
  }


  return score;
}

