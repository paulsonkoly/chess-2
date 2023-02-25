#include <stdio.h>
#include <strings.h>


#include "move.h"

void print_move_buffer(const MOVE * move, char * buffer) {
  SQUARE from = ffsl(move->from) - 1;
  SQUARE to   = ffsl(move->to) - 1;

  SQUARE ff = from & 7, fr = from >> 3, tf = to & 7, tr = to >> 3;

  buffer[0] = 'a' + ff;
  buffer[1] = '1' + fr;
  buffer[2] = 'a' + tf;
  buffer[3] = '1' + tr;

  if (move->special & PROMOTION_MOVE_MASK) {
    const char * p = "  nbrq";

    buffer[4] = p[(move->special & PROMOTION_MOVE_MASK) >> PROMOTION_MOVE_SHIFT];
    buffer[5] = 0;
  } else {
    buffer[4] = 0;
  }
}

void print_move(const MOVE * move) {
  char buffer[6];

  print_move_buffer(move, buffer);

  printf("%s", buffer);
}

