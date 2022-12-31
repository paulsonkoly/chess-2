#include "transposition.h"

#include <stdlib.h>
#include <stdio.h>


unsigned long tt_hitcnt;
unsigned long tt_misscnt;

typedef struct __ENTRY__ {
#define TT_LANE1_VALID 0x00000001
#define TT_LANE2_VALID 0x00000002
  unsigned int flags;
  TT_RESULT lane[2];
} ENTRY;

#define TT_SIZE 262144
ENTRY * table;

void initialize_tt() {
  if (NULL == (table = calloc(TT_SIZE, sizeof(ENTRY)))) {
    abort();
  }

  tt_reset();
}

void tt_reset() {
  int i;

  tt_hitcnt = 0;
  tt_misscnt = 0;

  for (i = 0; i < TT_SIZE; ++i) {
    table[i].flags = 0;
  }
}

void tt_free() {
  free(table);
}

const TT_RESULT * tt_probe(HASH hash, int depth) {
  unsigned int index = hash % TT_SIZE;
  int i;

  for (i = 0; i < 2; ++i) {
    if ((table[index].flags & (1 << i)) && table[index].lane[i].hash == hash && table[index].lane[i].depth >= depth) {
      tt_hitcnt++;
      return & table[index].lane[i];
    }
  }

  tt_misscnt++;

  return NULL;
}

void tt_info() {
  printf("info tt hits %ld misses %ld\n", tt_hitcnt, tt_misscnt);
}

void tt_insert_or_replace(HASH hash, int depth, int score, TT_TYPE type) {
  unsigned int index = hash % TT_SIZE;

  if ((! (table[index].flags & TT_LANE1_VALID)) || table[index].lane[0].depth < depth) {
    table[index].flags |= TT_LANE1_VALID;
    table[index].lane[0].hash  = hash;
    table[index].lane[0].type  = type;
    table[index].lane[0].score = score;
    table[index].lane[0].depth = depth;
  } else {
    table[index].flags |= TT_LANE2_VALID;
    table[index].lane[1].hash  = hash;
    table[index].lane[1].type  = type;
    table[index].lane[1].score = score;
    table[index].lane[1].depth = depth;
  }
}
