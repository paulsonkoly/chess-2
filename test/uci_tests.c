#include "./uci_tests.h"

void uci_parser_test1(void **state) {
  const char * line = "uci\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(UCI, cmd->type);

  uci_free(cmd);
}

void uci_parser_test2(void **state) {
  const char * line = "ucinewgame\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(UCINEWGAME, cmd->type);

  uci_free(cmd);
}

void uci_parser_test3(void **state) {
  const char * line = "isready\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(IS_READY, cmd->type);

  uci_free(cmd);
}

void uci_parser_test4(void **state) {
  const char * line = "go depth 10\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(GO, cmd->type);
  assert_int_equal(DEPTH, cmd->data.go.type);
  assert_int_equal(10, cmd->data.go.data.depth);

  uci_free(cmd);
}

void uci_parser_test5(void **state) {
  const char * line = "go perft 10\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(GO, cmd->type);
  assert_int_equal(PERFT, cmd->data.go.type);
  assert_int_equal(10, cmd->data.go.data.depth);

  uci_free(cmd);
}

void uci_parser_test6(void **state) {
  const char * line = "go movetime 5000000\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(GO, cmd->type);
  assert_int_equal(MOVETIME, cmd->data.go.type);
  assert_int_equal(5000000, cmd->data.go.data.movetime);

  uci_free(cmd);
}

void uci_parser_test7(void **state) {
  const char * line = "go wtime 5000000 winc 1000 btime 3000000 binc 1000\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(GO, cmd->type);
  assert_int_equal(WBTIME, cmd->data.go.type);
  assert_int_equal(5000000, cmd->data.go.data.wb_time.wtime);
  assert_int_equal(1000, cmd->data.go.data.wb_time.winc);
  assert_int_equal(3000000, cmd->data.go.data.wb_time.btime);
  assert_int_equal(1000, cmd->data.go.data.wb_time.binc);

  uci_free(cmd);
}

void uci_parser_test8(void **state) {
  const char * line = "go infinite\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(GO, cmd->type);
  assert_int_equal(INFINITE, cmd->data.go.type);

  uci_free(cmd);
}

void uci_parser_test9(void **state) {
  const char * line = "position fen rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(POSITION, cmd->type);
  assert_int_equal(FEN, cmd->data.position.type);
  assert_string_equal("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
      cmd->data.position.data.fen);

  uci_free(cmd);
}

void uci_parser_test10(void **state) {
  const char * line = "position fen rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(POSITION, cmd->type);
  assert_int_equal(FEN, cmd->data.position.type);
  assert_string_equal("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",
      cmd->data.position.data.fen);

  uci_free(cmd);
}

void uci_parser_test11(void **state) {
  const char * line = "position startpos moves e2e4 e7e5\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(POSITION, cmd->type);
  assert_int_equal(STARTPOS, cmd->data.position.type);
  assert_string_equal("e2e4 e7e5", cmd->data.position.data.moves);

  uci_free(cmd);
}

void uci_parser_test12(void **state) {
  const char * line = "position startpos\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(POSITION, cmd->type);
  assert_int_equal(STARTPOS, cmd->data.position.type);
  assert_null(cmd->data.position.data.moves);

  uci_free(cmd);
}

void uci_parser_test13(void **state) {
  const char * line = "position fen rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2 moves e2e4 e7e5\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(POSITION, cmd->type);
  assert_int_equal(FEN, cmd->data.position.type);
  assert_string_equal("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2", cmd->data.position.data.fen);
  assert_string_equal("e2e4 e7e5", cmd->data.position.data.moves);

  uci_free(cmd);
}

void uci_parser_test14(void **state) {
  const char * line = "position fen 1b3rk1/5ppp/2p2rq1/1p1n5/3P2P1/1BPbBP2/1P1N2QP/R3R1K1 w - - 0 1\n";
  UCI_CMD * cmd = uci_parse(line);

  assert_int_equal(POSITION, cmd->type);
  assert_int_equal(FEN, cmd->data.position.type);
  assert_string_equal("1b3rk1/5ppp/2p2rq1/1p1n5/3P2P1/1BPbBP2/1P1N2QP/R3R1K1 w - - 0 1", cmd->data.position.data.fen);
  assert_null(cmd->data.position.data.moves);

  uci_free(cmd);
}
