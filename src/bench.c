#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "killer.h"
#include "search.h"
#include "pv.h"

static const char * fens [] = {
  /* openings */
  "r1bqkb1r/ppp1p1pp/5n2/3p4/1nBP4/2N2N2/PPP3PP/R1BQ1RK1 w kq - 0 9",
  "rnbq1rk1/p1p1b1p1/1p2pn1p/3p1pB1/3P4/3BPN2/PPP1NPPP/R2Q1RK1 w - - 0 9",
  "r2qk2r/pbpp1ppp/1p3n2/1B2p3/1b2P3/2NP1Q2/PPPB1PPP/R4RK1 w kq - 3 9",
  "r2qk2r/ppp1ppbp/4bnp1/6N1/2pn4/N5P1/PP2PPBP/R1BQ1RK1 w kq - 0 9",
  "rn1qkbnr/6pp/4p3/2pp4/4P3/2N5/PP3PPP/R1BQKbNR w KQkq - 0 9",
  "r1bqk2r/pppnnpbp/3p2p1/3Pp3/2P1P3/2N1BN2/PP3PPP/R2QKB1R w KQkq - 1 9",
  "rnbqk2r/pp2ppbp/5np1/8/3N4/2N3P1/PP2PPBP/R1BQK2R w KQkq - 2 9",
  "r1bq1rk1/pppn1pbp/3p2p1/3Pp2n/2P1P3/2N1BN2/PP2BPPP/R2QK2R w KQ - 1 9",
  "r3k1nr/pp1b1pp1/1qn1p3/2bpP2p/8/2PB1N2/PPQ2PPP/RNB1K2R w KQkq h6 0 9",
  "r1bqkb1r/pp1n1p1p/2n1p1p1/2ppP3/3P4/2P2N1P/PP2NPP1/R1BQKB1R w KQkq - 1 9",

  /* middle game (sampled from http://www.talkchess.com/forum3/viewtopic.php?f=2&t=70438) */
  "2r2rk1/1b3p1p/p5p1/q2pb3/Pp6/3BP2P/1P1NQPP1/3R1RK1 w - - 0 0",
  "1r1r2k1/1p2qppp/pN2bn2/P7/2Nnp3/6PP/1P1RPPB1/4QRK1 w - - 0 0",
  "r4rk1/1q2bp1p/p1bpp1p1/4P2n/3B1P2/3B3Q/1PP1N1PP/4RR1K w - - 0 0",
  "1r2k2r/1bqppp1p/5np1/1pP5/1R1Q4/2P2NP1/4PPBP/4K2R w Kk - 0 0",
  "2r2rk1/1b3p1p/1p1q1np1/pPnp4/3N4/P1N1P1P1/5PBP/R2Q1RK1 w - - 0 0",
  "1r2rbk1/2qp1ppp/2p1p2B/3nP3/p6P/1P4Q1/P1P1BPP1/R2R2K1 w - - 0 0",
  "r2qr1k1/pp1n1pb1/3P2pp/2p2b2/1nQ2B2/2N2N2/PP1RBPPP/R5K1 w - - 0 0",
  "r1bq3k/1ppnn2p/3p1rpb/P2Ppp2/1BP1P3/2NB4/3N1PPP/R2Q1RK1 w - - 0 0",
  "1r2kn1r/2p1p1b1/p2pp2p/6p1/P1qP4/1pP2N2/1P1B1PPP/R2QR1K1 w k - 0 0",
  "1r2k2r/1p3ppp/p2bp3/4n3/4B3/1NP5/P1P2PPP/R3K2R w KQk - 0 0",

  /* endgames (sampled from https://www.stmintz.com/ccc/index.php?id=476109) */
  "4b3/5k2/4p1p1/3pP2p/2pP1P1P/2P5/6N1/2K5 w - - 0 0",
  "3r4/7p/Rp4k1/5p2/4p3/2P5/PP3P1P/5K2 b - - 0 0",
  "4b3/pp2kp1p/5bp1/8/8/4P3/PP2NPPP/3N2K1 w - - 0 0",
  "3R4/1p6/2b5/2P1k2p/p3p2P/P6r/1P2KB2/8 b - - 0 0",
  "2r3k1/4pp1p/3p2p1/p2R4/1r6/1P4P1/P3PP1P/R5K1 b - - 0 0",
  "6k1/2p3np/1p1p2p1/3P4/1PPK1R2/6PB/7P/4r3 w - - 0 0",
  "8/5b2/1k2p1p1/2NpP1P1/1K1P4/8/8/8 w - - 0 0",
  "1n6/4k2p/p3ppp1/1pPp4/3P1PP1/3NP3/P3K2P/8 w - - 0 0",
  "8/1pk3pp/p7/3p1p2/8/3K2P1/PP2PP1P/8 w - - 0 0",
  "8/1ppr1kp1/p1p4p/8/8/5P2/PPP1RKPP/8 w - - 0 0",
};

static struct timespec label_start[3];
static struct timespec label_end[3];
unsigned long long nodes_per_gamephase[3];

static void bench_run(BOARD * b);

void bench() {
  int i;
  const char * labels[] = { "openings", "middle games", "end games" };
  BOARD * boards[30];
  unsigned long long milisecs_overall = 0;
  unsigned long long nodes_overall = 0;

  for (i = 0; i < 30; ++i) {
    if (NULL == (boards[i] = parse_fen(fens[i]))) {
      abort();
    }
  }

  for (i = 0; i < 30; ++i) {
    int phase = i / 10;
    if (i % 10 == 0) {
      printf("%s", labels[phase]);
      nodes = 0;
      nodes_per_gamephase[phase] = 0;

      if (clock_gettime(CLOCK_REALTIME, label_start + (i / 10))) {
        printf("info clock_gettime failed\n");
      }
    }

    bench_run(boards[i]);

    if (i % 10 == 9) {
      int phase = i / 10;
      unsigned long long milisecs;
      if (clock_gettime(CLOCK_REALTIME, label_end + phase)) {
        printf("info clock_gettime failed\n");
      }
      nodes_per_gamephase[phase] = nodes;
      milisecs = (label_end[phase].tv_sec - label_start[phase].tv_sec) * 1000 +
        (label_end[phase].tv_nsec - label_start[phase].tv_nsec) / 1000000;

      printf("\ttime %llu nodes %llu nps %llu\n", milisecs, nodes, nodes * 1000 / milisecs);
      milisecs_overall += milisecs;
      nodes_overall+= nodes;
    }
  }

  for (i = 0; i < 30; ++i) {
    free(boards[i]);
  }

  printf("nodes %llu nps %llu\n", nodes_overall, nodes_overall * 1000 / milisecs_overall);
}

/* version of iterative deepening with a lot of functionality removed, and not resetting node */
static void bench_run(BOARD * board) {
  PV * opv; /* old PV */
  PV * npv; /* next PV */
  KILLER killer;
  const int max_depth = 8;

  if ((NULL == (opv = pv_init())) || (NULL == (npv = pv_init()))) {
    abort();
  }

  for (int depth = 1; depth <= max_depth; ++depth) {
    pv_reset(npv);
    reset_killer(&killer);

    (void)negascout(board, 0, depth, depth , -10000, 10000, opv, &npv, &killer);

    pv_swap(&opv, &npv);
  }

  pv_destroy(opv);
  pv_destroy(npv);
}

