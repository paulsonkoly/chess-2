#!/usr/bin/env bash
#
# debug wrapper for https://github.com/agausmann/perftree

DEPTH="$1"
FEN="$2"
MOVES="$3"
TMP=`mktemp`

if [[ ! -z "$MOVES" ]]; then
  MOVES="moves $MOVES"
fi

./chess2 >$TMP <<eos
position fen $FEN $MOVES
go perft $DEPTH
eos


head -n -1 $TMP | cut -f7-8 -d\ 
echo
tail -n 1 $TMP | sed "s/ //g"

rm $TMP
