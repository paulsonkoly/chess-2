source scripts/progress.sh

BRANCH=`git rev-parse --abbrev-ref HEAD`
make clean
make

echo Running chess2 bench 30x
rm -f "${BRANCH}_bench_results.txt"

for (( i = 0; i < 30; i++ )); do
  ProgressBar $i 29
  echo `./chess2 bench | tail -n 2 | tr '\n' ' '` >> "${BRANCH}_bench_results.txt"
done
echo

echo Checking out main
git checkout main
make clean
make

rm -f "main_bench_results.txt"
for (( i = 0; i < 30; i++ )); do
  ProgressBar $i 29
  echo `./chess2 bench | tail -n 2 | tr '\n' ' '` >> "main_bench_results.txt"
done
echo

echo Checking out $BRANCH
git checkout "$BRANCH"

echo Comparing times
ministat -C 1 -w 50 "main_bench_results.txt" "${BRANCH}_bench_results.txt"
echo Comparing nps
ministat -C 5 -w 50 "main_bench_results.txt" "${BRANCH}_bench_results.txt"
