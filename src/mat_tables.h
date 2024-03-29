#ifndef __MAT_TABLES_H__
#define __MAT_TABLES_H__

#include <stdint.h>

#include "chess.h"
#include "board.h"

typedef struct __MAT_TABLE_ENTRY__ {
#define ENDGAME_MASK        0x00000003
#define ENDGAME_0           0x00000000 /* opening */
#define ENDGAME_1           0x00000001 /* early middle game */
#define ENDGAME_2           0x00000002 /* late middle game */
#define ENDGAME_3           0x00000003 /* endgame */
#define DRAWN               0x00000004 /* insufficient material */
/* the following W/B counterparts have to be 1 bit position apart*/
#define BN_MATE_LSQ         0x00000008 /* deliver bishop / knight checkmate to black in light sq corner */
#define BN_MATE_DSQ         0x00000010
#define W_CHECKMATING       0x00000020
#define B_CHECKMATING       0x00000040
#define CONSTRAINT          0x10000000 /* internal use for mat tables */
  int value;
  uint32_t flags;
} MAT_TABLE_ENTRY;

extern MAT_TABLE_ENTRY * mat_table;

void initialize_mat_tables(void);
const MAT_TABLE_ENTRY * get_mat_table_entry(const BOARD * board);

#if DEBUG
void mat_table_debug(const BOARD * board);
#define MAT_TABLE_DEBUG(board) mat_table_debug(board)
#else
#define MAT_TABLE_DEBUG(board)
#endif

#endif /* ifndef __MAT_TABLES_H__ */
