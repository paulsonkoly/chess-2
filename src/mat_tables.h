#ifndef __MAT_TABLES_H__
#define __MAT_TABLES_H__

#include <stdint.h>

#include "board.h"

typedef struct __MAT_TABLE_ENTRY__ {
/* #define ENDGAME_FACTOR_MASK 0x00000003 */
#define DRAWN               0x00000004 /* insufficient material */
  int value;
  uint32_t flags;
} MAT_TABLE_ENTRY;

extern MAT_TABLE_ENTRY * mat_table;

void initialize_mat_tables();
const MAT_TABLE_ENTRY * get_mat_table_entry(const BOARD * board);

#endif /* ifndef __MAT_TABLES_H__ */
