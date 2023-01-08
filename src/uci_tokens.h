#ifndef __UCI_TOKENS__
#define __UCI_TOKENS__

#include <stdlib.h>

typedef struct yy_buffer_state *YY_BUFFER_STATE;
typedef size_t yy_size_t;
extern YY_BUFFER_STATE yy_scan_string(const char * string);
extern int yylex (void);
char *yyget_text ( void );

enum UCI_TOKEN {
  TOK_UCINEWGAME = 0,
  TOK_UCI,
  TOK_IS_READY,
  TOK_STOP,
  TOK_GO,
  TOK_DEPTH,
  TOK_PERFT,
  TOK_MOVETIME,
  TOK_INFINITE,
  TOK_WTIME,
  TOK_WINC,
  TOK_BTIME,
  TOK_BINC,
  TOK_POSITION,
  TOK_FEN,
  TOK_FEN_STRING,
  TOK_STARTPOS,
  TOK_MOVES,
  TOK_MOVES_STRING,
  TOK_NUMBER,
  TOK_EOL
};

#endif /* ifndef __UCI_TOKENS__ */
