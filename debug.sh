#!/usr/bin/env bash

SELF="./chess2"
STOCKFISH=stockfish

STOCKFISHTMP=`mktemp /tmp/XXXXX`
($STOCKFISH | grep ': ' | sed "s/Nodes searched: //" | tr -d ":" | sort -k 1 > $STOCKFISHTMP) <<HEREDOC
  position fen $2
  go perft $1
HEREDOC

MYTEMP=`mktemp /tmp/XXXXX`
($SELF | sort -k 7 > $MYTEMP) << HEREDOC
position fen $2
go perft $1
HEREDOC
MYTEMPRESULTS=`mktemp /tmp/XXXXX`
cut -f7-9 -d\  $MYTEMP > $MYTEMPRESULTS

join -j 1 -o 1.1 1.2 2.1 2.2 $MYTEMPRESULTS $STOCKFISHTMP | tail -n +2 | while read LINE; do
  MY=`echo $LINE | cut -f2 -d\ `
  ST=`echo $LINE | cut -f4 -d\ `

  if [[ "$MY" != "$ST" ]]; then
    MOVE=`echo $LINE | cut -f1 -d\ `
    FEN=`grep $MOVE $MYTEMP | cut -f1-6 -d\ `

    echo "Debugging $LINE $FEN"
    $0 "$((${1} - 1))" "$FEN"
    break
  fi
done

rm -rf $MYTEMPRESULTS $MYTEMP $STOCKFISHTMP

