#ifndef _UCI_H_
#define _UCI_H_

#include "uci_tokens.h"

#define MOVE_BUFFER_LEN 1024

enum UCI_TYPE { INVALID = 0, UCI = TOK_UCI, UCINEWGAME = TOK_UCINEWGAME, IS_READY = TOK_IS_READY, GO = TOK_GO, POSITION = TOK_POSITION };
enum UCI_GO_TYPE { INFINITE, DEPTH = TOK_DEPTH, MOVETIME = TOK_MOVETIME, WBTIME, PERFT = TOK_PERFT };
enum UCI_POSITION_TYPE { FEN = TOK_FEN, STARTPOS = TOK_STARTPOS };

typedef struct _UCI_CMD_ {

  enum UCI_TYPE type;
  union DATA {

    struct GO {

      enum UCI_GO_TYPE type;
      union GO_DATA {
        int depth;

        unsigned long long movetime;

        struct WB_TIME {
          unsigned long long wtime;
          unsigned long long winc;
          unsigned long long btime;
          unsigned long long binc;
        } wb_time;

      } data;
    } go;

    struct POSITION {

      enum UCI_POSITION_TYPE type;
      union POSITION_DATA {
        char * fen;
        char * moves;
      } data;
    } position;

  } data;
} UCI_CMD;

UCI_CMD * uci_parse(const char * line);
void uci_free(UCI_CMD * cmd);
void uci();

#endif /* ifndef _UCI_H_ */
