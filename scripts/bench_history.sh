#!/usr/bin/env bash

source scripts/progress.sh

FAKE=`mktemp -d`
GITLOG=`mktemp`
LOG=`mktemp`
HISTORY=`mktemp -d`
RESULTS=`mktemp`

git clone https://github.com/phaul/chess-2 $FAKE > $LOG
cd $FAKE

git log --pretty=oneline 00e30eeb3d7908..main > $GITLOG
count=1
total=`wc -l $GITLOG`

ProgressBar 0 $total
while read SHA MESSAGE; do
  git checkout $SHA >> $LOG 2>&1
  make clean >> $LOG
  make >> $LOG
  # in search we read stdin for stop commands, which would read off the rest of the loop input
  echo "" | ./chess2 bench > $HISTORY/$SHA
  ProgressBar $((count++)) $total
done < $GITLOG

echo

# seq number - nps - sha - message
tac $GITLOG | cat -n | while read SEQ SHA MESSAGE; do
   echo $SEQ `cat $HISTORY/$SHA | tail -n1 | cut -f3 -d' '` $SHA $MESSAGE
done > $RESULTS

echo Results in $RESULTS
echo Logs in $LOG

gnuplot <<EOF
set terminal dumb size 120, 30
plot '$RESULTS' using 1:2:1 with labels notitle
EOF

while read NO NPS SHA MESSAGE; do
  echo "[$NO] $SHA nps: $NPS - $MESSAGE"
done < $RESULTS

rm -rf $FAKE
rm -rf $HISTORY
rm -f $GITLOG
rm -f $HISTORY
