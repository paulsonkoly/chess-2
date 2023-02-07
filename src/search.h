#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "board.h"
#include "killer.h"
#include "pv.h"

extern unsigned long long nodes;

int negascout(BOARD* board,
    int ply,
    int depth,
    int reduced_depth,
    int alpha,
    int beta,
    const PV * opv,
    PV ** npv,
    KILLER * killer);

enum SEARCH_LIMIT_TYPE { SL_INFINITE, SL_DEPTH, SL_MOVETIME, SL_WBTIME };
typedef struct __SEARCH_LIMIT__ {
  enum SEARCH_LIMIT_TYPE type;

  union {
    int depth;
    unsigned long long movetime;
    struct WB_TIME {
      unsigned long long wtime;
      unsigned long long winc;
      unsigned long long btime;
      unsigned long long binc;
    } wb_time;
  } data;
} SEARCH_LIMIT;

int iterative_deepening(BOARD * board, const SEARCH_LIMIT* search_limit);

#endif /* ifndef _SEARCH_H_ */
