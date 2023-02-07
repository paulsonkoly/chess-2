#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "uci.h"
#include "uci_tokens.h"
#include "board.h"
#include "search.h"
#include "evaluate.h"
#include "mat_tables.h"
#include "perft.h"

UCI_CMD * uci_parse(const char * line) {
  UCI_CMD * cmd;
  enum UCI_TOKEN tok;

  if (NULL == (cmd = malloc(sizeof(UCI_CMD)))) {
    return NULL;
  }

  cmd->type = INVALID;
  cmd->data.go.data.wb_time.wtime = 0;
  cmd->data.go.data.wb_time.winc = 0;
  cmd->data.go.data.wb_time.btime = 0;
  cmd->data.go.data.wb_time.binc = 0;

  (void)yy_scan_string(line);

  switch (tok = yylex()) {
    case TOK_UCINEWGAME:
    case TOK_UCI:
    case TOK_IS_READY:
    case TOK_STOP:
    case TOK_QUIT:
#if DEBUG
    case TOK_EVALUATE:
    case TOK_MATTABLE:
#endif
      cmd->type = (enum UCI_TYPE)tok; return cmd;

    case TOK_GO:
      cmd->type = GO;

      while (tok != TOK_EOL) {

        switch (tok = yylex()) {

          case TOK_DEPTH: case TOK_PERFT:
            cmd->data.go.type = (enum UCI_GO_TYPE)tok;
            if (yylex() == TOK_NUMBER) {
              cmd->data.go.data.depth = atoi(yyget_text());
            }

            return cmd;

          case TOK_MOVETIME:
            cmd->data.go.type = (enum UCI_GO_TYPE)tok;
            if (yylex() == TOK_NUMBER) {
              cmd->data.go.data.movetime = atoll(yyget_text());
            }

            return cmd;

          case TOK_INFINITE: cmd->data.go.type = INFINITE; return cmd;

          case TOK_WTIME: case TOK_WINC: case TOK_BTIME: case TOK_BINC:
            cmd->data.go.type = WBTIME;
            if (yylex() == TOK_NUMBER) {
              unsigned long long * ptr = (& cmd->data.go.data.wb_time.wtime) + tok - TOK_WTIME;
              *ptr = atoll(yyget_text());
            }
            break;

          default: ;
        }
      }

      break;

    case TOK_POSITION:
      cmd->type = POSITION;
      cmd->data.position.data.moves = NULL;

        switch (yylex()) {
          case TOK_FEN:
            cmd->data.position.type = FEN;
            if (TOK_FEN_STRING == yylex()) {
              cmd->data.position.data.fen = strdup(yyget_text());
            }
            break;

          case TOK_STARTPOS:
            cmd->data.position.type = STARTPOS;
            break;

          default: ;
        }

        if (yylex() == TOK_MOVES && yylex() == TOK_MOVES_STRING) {
          cmd->data.position.data.moves = strdup(yyget_text());
        }

        break;

    default: ;
    }

  return cmd;
}

void uci_free(UCI_CMD * cmd) {
  if (cmd->type == POSITION) {
    if (cmd->data.position.type == FEN) {
      free(cmd->data.position.data.fen);
    }
    if (cmd->data.position.type == STARTPOS && cmd->data.position.data.moves) {
      free(cmd->data.position.data.moves);
    }
  }

  free(cmd);
}

void uci(void) {
  BOARD * board;

  board = initial_board();

  while (1) {
    char * line = NULL;
    size_t count;

    if (-1 == getline(&line, &count, stdin)) {
      exit(0);
    }

    UCI_CMD * cmd;

    if (NULL == (cmd = uci_parse(line))) {
      abort();
    }

    switch (cmd->type) {

      case UCI:
#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)
        printf("id name chess2 (%s)\n", EXPAND_AND_QUOTE(VERSION));
        printf("id author Paul Sonkoly\n");
        printf("uciok\n");
        break;

      case IS_READY:
        printf("readyok\n");
        break;

      case STOP:
        break;

      case GO: {
        SEARCH_LIMIT limit;

        switch (cmd->data.go.type) {

          case DEPTH:
            limit.type = SL_DEPTH;
            limit.data.depth = cmd->data.go.data.depth;
            iterative_deepening(board, &limit);
            break;

          case PERFT:
            perft(board, cmd->data.go.data.depth, 1);
            break;

          case MOVETIME:
            limit.type = SL_MOVETIME;
            limit.data.movetime = cmd->data.go.data.movetime;
            iterative_deepening(board, &limit);
            break;

          case INFINITE:
            limit.type = SL_INFINITE;
            iterative_deepening(board, &limit);
            break;

          case WBTIME:
            limit.type = SL_WBTIME;
            limit.data.wb_time.wtime = cmd->data.go.data.wb_time.wtime;
            limit.data.wb_time.winc = cmd->data.go.data.wb_time.winc;
            limit.data.wb_time.btime = cmd->data.go.data.wb_time.btime;
            limit.data.wb_time.binc = cmd->data.go.data.wb_time.binc;
            iterative_deepening(board, &limit);
            break;

        }
        break;
      }

      case POSITION:
        switch (cmd->data.position.type) {

          case FEN:
            free(board);
            board = parse_fen(cmd->data.position.data.fen);

            break;

          case STARTPOS:
            free(board);
            board = initial_board();

            break;
        }

        if (cmd->data.position.data.moves != NULL) {
          play_uci_moves(board, cmd->data.position.data.moves);
        }
        break;

#if DEBUG
      case EVAL:
        DEBUG_ENABLE;
        evaluate(board);
        DEBUG_DISABLE;
        break;

      case MATTABLE:
        mat_table_debug(board);
        break;
#endif

      case QUIT:
        fflush(stdout);

        free(board);
        free(line);
        uci_free(cmd);

        return;

      default:;
    }

    fflush(stdout);

    free(line);
    uci_free(cmd);

  }
}

