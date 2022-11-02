#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "uci.h"
#include "board.h"
#include "search.h"
#include "perft.h"

extern unsigned long long movetime;

UCI_CMD * uci_parse(const char * line) {
  UCI_CMD * cmd;

  if (NULL == (cmd = malloc(sizeof(UCI_CMD)))) {
    return NULL;
  }

  if (strncmp(("uci"), line, strlen("uci")) == 0) {
    cmd->type = UCI;

    return cmd;
  }

  if (strncmp(("isready"), line, strlen("isready")) == 0) {
    cmd->type = IS_READY;

    return cmd;
  }

  if (strncmp(("go"), line, strlen("go")) == 0) {
    cmd->type = GO;
    line += strlen("go") + 1;

    if (strncmp(("depth"), line, strlen("depth")) == 0) {
      line += strlen("depth") + 1;
      cmd->data.go.type = DEPTH;
      cmd->data.go.data.depth = atoi(line);
    }

    if (strncmp(("perft"), line, strlen("perft")) == 0) {
      line += strlen("perft") + 1;
      cmd->data.go.type = PERFT;
      cmd->data.go.data.depth = atoi(line);
    }

    if (strncmp(("movetime"), line, strlen("movetime")) == 0) {
      line += strlen("movetime") + 1;
      cmd->data.go.type = MOVETIME;
      cmd->data.go.data.movetime = atoll(line);
    }

    return cmd;
  }

  if (strncmp(("position"), line, strlen("position")) == 0) {
    cmd->type = POSITION;
    line += strlen("position") + 1;

    if (strncmp(("fen"), line, strlen("fen")) == 0) {
      line += strlen("fen") + 1;
      cmd->data.position.type = FEN;
      cmd->data.position.data.fen = line;
    }

    if (strncmp(("startpos"), line, strlen("startpos")) == 0) {
      line += strlen("startpos") + 1;
      cmd->data.position.type = STARTPOS;

      if (strncmp(("moves"), line, strlen("moves")) == 0) {
        line += strlen("moves") + 1;
        cmd->data.position.data.moves = line;
      }
      else {
        cmd->data.position.data.moves = NULL;
      }
    }
    return cmd;
  }

  cmd->type = INVALID;

  return cmd;
}

void uci() {
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
          printf("id name chess2\n");
          printf("id author Paul Sonkoly\n");
          printf("uciok\n");
          break;

      case IS_READY:
          printf("readyok\n");
          break;

      case GO:
          movetime = 0;

          switch (cmd->data.go.type) {

            case DEPTH:
              iterative_deepening(board, cmd->data.go.data.depth);
              break;

            case PERFT:
              perft(board, cmd->data.go.data.depth, 1);
              break;


            case MOVETIME:
              movetime = cmd->data.go.data.movetime;

            case INFINITE:
              iterative_deepening(board, 1000);
              break;

          }
          break;

      case POSITION:
          switch (cmd->data.position.type) {

            case FEN:
              free(board);
              board = parse_fen(cmd->data.position.data.fen);

              break;

            case STARTPOS:
              free(board);
              board = initial_board();
              if (cmd->data.position.data.moves != NULL) {
                play_uci_moves(board, cmd->data.position.data.moves);
              }

              break;
          }
          break;

      default:;
    }

    fflush(stdout);

    free(line);
    free(cmd);

  }
}

