#include <stdio.h>

#include "move.h"

void print_move(const MOVE * move) {
  SQUARE from = ffsl(move->from) - 1;
  SQUARE to   = ffsl(move->to) - 1;

  SQUARE ff = from & 7, fr = from >> 3, tf = to & 7, tr = to >> 3;

  printf("%c%c%c%c", 'a' + ff, '1' + fr, 'a' + tf, '1' + tr);

  if (move->promotion) {
    const char * p = "  nbrq";

    printf("%c", p[move->promotion]);
  }
}
