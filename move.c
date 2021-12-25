#include <stdio.h>

#include "move.h"

void print_move(const MOVE * move) {
  if (IS_CASTLE & move->castle) {
    CASTLE c = ALL_CASTLES & move->castle;
    const char * cs = "e1g1\0e1c1\0e8g8\0e8c8";

    printf("%s", &cs[c*5]);
  } else {
    SQUARE from = move->from;
    SQUARE to   = move->to;

    SQUARE ff = from & 7, fr = from >> 3, tf = to & 7, tr = to >> 3;

    printf("%c%c%c%c", 'a' + ff, '1' + fr, 'a' + tf, '1' + tr);

    if (move->promotion) {
      const char * p = "  nbrq";

      printf("%c", p[move->promotion]);
    }
  }
}
