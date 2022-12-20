#include "see.h"

#include "movelist.h"
#include "moveexec.h"
#include "movegen.h"

static const int piece_value[] = {0, 100, 325, 400, 500, 900, 10000};

static int see__(BOARD * board, const MOVE * move) {
  int value = 0;

  MOVE * response;

  if ((response = add_least_valuable_attacker(board, move))) {
    PIECE capture = (response->special & CAPTURED_MOVE_MASK) >> CAPTURED_MOVE_SHIFT;

    execute_move(board, response);

    value = piece_value[capture] - see__(board, response);

    undo_move(board, response);

    value = value < 0 ? 0 : value;
  }

  return value;
}

int see(BOARD * board, const MOVE * move) {
  PIECE capture = (move->special & CAPTURED_MOVE_MASK) >> CAPTURED_MOVE_SHIFT;

  int value = piece_value[capture];

  ml_open_frame();

  execute_move(board, move);

  value = piece_value[capture] - see__(board, move);

  undo_move(board, move);

  ml_close_frame();

  return value;
}
