#ifndef _UCI_H_
#define _UCI_H_

#define MOVE_BUFFER_LEN 1024

enum UCI_TYPE { INVALID, UCI, UCINEWGAME, IS_READY, GO, POSITION };
enum UCI_GO_TYPE { INFINITE, DEPTH, MOVETIME, PERFT };
enum UCI_POSITION_TYPE { FEN, STARTPOS };

typedef struct _UCI_CMD_ {

  enum UCI_TYPE type;
  union DATA {

    struct GO {

      enum UCI_GO_TYPE type;
      union GO_DATA {
        int depth;
        unsigned long long movetime;
      } data;
    } go;

    struct POSITION {

      enum UCI_POSITION_TYPE type;
      union POSITION_DATA {
        const char * fen;
        const char * moves;
      } data;
    } position;

  } data;
} UCI_CMD;

UCI_CMD * uci_parse(const char * line);

void uci();

#endif /* ifndef _UCI_H_ */
