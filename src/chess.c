#include "chess.h"

int debug = 0;
const int piece_values[] = { NO_PIECE_V, PAWN_V, KNIGHT_V, BISHOP_V, ROOK_V, QUEEN_V, KING_V };
const char * piece_names[] = { "no piece", "pawn", "knight", "bishop", "rook", "queen", "king" };
const char * colour_names[] = { "white", "black" };
