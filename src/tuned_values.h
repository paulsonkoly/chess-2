#ifndef __TUNED_VALUES_H__
#define __TUNED_VALUES_H__

typedef struct __TUNED_VALUES__ {
  int piece_v_pawn_v;
  int piece_v_knight_v;
  int piece_v_bishop_v;
  int piece_v_rook_v;
  int piece_v_queen_v;

  int pawn_ranks_0;
  int pawn_ranks_1;
  int pawn_ranks_2;
  int pawn_ranks_3;
  int pawn_ranks_4;
  int pawn_ranks_5;
  int pawn_ranks_6;
  int pawn_ranks_7;

  int pawn_isolated;
  int pawn_weak;

  int king_shield;
} TUNED_VALUES;

#if defined(TUNING)
extern TUNED_VALUES tuned_values[4];
#else
extern const TUNED_VALUES tuned_values[4];
#endif

#endif /* ifndef __TUNED_VALUES_H__ */
