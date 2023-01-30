#ifndef _MOVELIST_H_
#define _MOVELIST_H_

#include "move.h"

/* very simple move store, with no fragmentation. We support allocating plys
 * and moves within plys. Active ply is the last one opened, and stays active
 * until closed. Once a ply is closed its moves are freed. ml_first() points to
 * the first ml_last() points to the last allocated move in active ply.
 */

/* we allow 2k moves for 64 plies it allows for 32 moves per ply on average.
 * However we don't allocate all moves for all plys and we only wish to be able
 * to go 64 (MAX_PLIES) plies
 */
#define MAX_MOVES 2048

/* opens a frame for a new ply */
void ml_open_frame(void);

/* closes the frame last opened */
void ml_close_frame(void);

/* first move within the active frame, if called on empty frame or when no
 * frames are opened behaviour is undefined */
MOVE * ml_first(void);

/* points after the last move within the active frame, if called on empty frame
 * or when no frames are opened behaviour is undefined  */
MOVE * ml_last(void);

/* allocates the next move */
MOVE * ml_allocate(void);

#endif /* ifndef _MOVELIST_H_ */
