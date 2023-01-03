# chess-2

Hi, I am a UCI chess engine. I'm the second chess engine my author has written, thus the really unimaginative name, following the first program [chess](https://github.com/phaul/chess). You can find my [lichess profile](https://lichess.org/@/chess-2-bot). Challenge me to a game if you fancy.

# Running the engine

Normally a UCI chess engines requires a GUI. Arena and Banksia are tested with chess-2, but all UCI compatible UIs should work.

You can also run the engine in the console, issuing [UCI](http://wbec-ridderkerk.nl/html/UCIProtocol.html) commands.

# Build

On a linux/unix compatible system with the GNU C compiler and GNU make the engine should build without any tweaks required:

     % cd chess-2
     % make
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I./src main.c -o main.o
      make[1]: Entering directory '/home/phaul/chess-2/src'
      flex -o uci.yy.c uci.fl
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src uci.yy.c -o uci.yy.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src attacks.c -o attacks.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src board.c -o board.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src evaluate.c -o evaluate.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src killer.c -o killer.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src move.c -o move.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src moveexec.c -o moveexec.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src movegen.c -o movegen.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src movelist.c -o movelist.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src pawns.c -o pawns.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src perft.c -o perft.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src pv.c -o pv.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src search.c -o search.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src see.c -o see.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src uci.c -o uci.o
      cc -c -Wall -Werror -pedantic -O2 -g -DNDEBUG -I/src zobrist.c -o zobrist.o
      make[1]: Leaving directory '/home/phaul/chess-2/src'
      cc -Wall -Werror -pedantic -O2 -g -DNDEBUG -I./src -o chess2 main.o src/*.o

# Testing

To build the CI tests install the [cmocka](https://cmocka.org/) testing framework.

     % cd chess-2/test
     % make
      make[1]: Entering directory '/home/phaul/chess-2/src'
      make[1]: Nothing to be done for 'build'.
      make[1]: Leaving directory '/home/phaul/chess-2/src'
      touch .engineobjs
      cc -o test main.o uci_tests.o zobrist_tests.o .engineobjs ../src/*.o -lcmocka 
     % ./test
      [==========] Running 30 test(s).
      [ RUN      ] perft_unit_test1
      [       OK ] perft_unit_test1
      [       OK ] uci_parser_test13

      ...

      [ RUN      ] uci_parser_test14
      [       OK ] uci_parser_test14
      [ RUN      ] zobrist_test1
      [       OK ] zobrist_test1
      [ RUN      ] zobrist_test2
      [       OK ] zobrist_test2
      [==========] 30 test(s) run.
      [  PASSED  ] 30 test(s).
      
# Contributing

New features are created on feature branches. Before a merge to main can happen the feature branch has to run against main using [cutechess-cli](https://github.com/cutechess/cutechess).

     cutechess-cli -each tc=5+3 proto=uci -engine cmd=./chess2_new -engine cmd=./chess2_old -games 128 -concurrency 8 -openings file=../OpenBench/Books/Pohl.epd -sprt alpha=0.05 beta=0.05 elo0=100 elo1=200

The branch should not be merged if there is a significant ELO drop.

## Performance

The `scripts/bench.sh` can be run against a feature branch which runs the engine with the `bench` argument. The script provides statistical analysis of of performance improvements.
