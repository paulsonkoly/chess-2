#ifndef _PV_H_
#define _PV_H_

#include "move.h"

/* principal variation */
typedef struct _PV_ PV;

/* allocate new PV */
PV * pv_init(void);
/* destroy PV */
void pv_destroy(PV* pv);
/* reset PV to empty */
void pv_reset(PV* pv);
/* swap 2 pointers */
void pv_swap(PV** pv1, PV** pv2);
/* insert move*/
void pv_insert(PV * pv, const MOVE * move, int offset);
/* retrieve move from PV */
MOVE * pv_getmove(const PV* PV, int offset);
/* number of moves in PV */
int pv_count(const PV * pv);

#endif /* ifndef _PV_H_ */
