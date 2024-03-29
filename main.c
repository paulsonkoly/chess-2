/*
chess 2
Copyright © 2021 Paul Sonkoly

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chess.h"
#include "attacks.h"
#include "uci.h"
#include "zobrist.h"
#include "bench.h"
#include "mat_tables.h"

void print_bitboard(BITBOARD bb) {
  printf("--------\n");
  for (SQUARE r = 56; r < 64; r -= 8) {
    for (SQUARE f = 0; f != 8; f += 1) {
      SQUARE sq = r | f;

      printf("%c", bb & ((BITBOARD)1 << sq ) ? 'X' : '.');
    }
    printf("\n");
  }
  printf("--------\n");
}

int stopped = 0;

int main(int argc, char *argv[])
{
#if DEBUG
  printf("DEBUG BUILD\n");
#endif

  initialize_magic();
  initialize_in_between();
  initialize_hash();
  initialize_mat_tables();

  if (argc == 2 && 0 == strncmp(argv[1], "bench", strlen("bench"))) {
    bench();
  }
  else {
    uci();
  }

  free(mat_table);

  return 0;
}

