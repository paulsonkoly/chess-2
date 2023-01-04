# https://stackoverflow.com/questions/238073/how-to-add-a-progress-bar-to-a-shell-script
function ProgressBar {
# Process data
    let _progress=(${1}*100/${2}*100)/100
    let _done=(${_progress}*4)/10
    let _left=40-$_done
# Build progressbar string lengths
    _fill=$(printf "%${_done}s")
    _empty=$(printf "%${_left}s")

# 1.2 Build progressbar strings and print the ProgressBar line
# 1.2.1 Output example:
# 1.2.1.1 Progress : [########################################] 100%
printf "\rProgress : [${_fill// /#}${_empty// /-}] ${_progress}%%"
}

BRANCH=`git rev-parse --abbrev-ref HEAD`
make

echo Running chess2 bench 30x
rm -f "${BRANCH}_bench_results.txt"

for (( i = 0; i < 30; i++ )); do
  ProgressBar $i 30
  echo `./chess2 bench | tail -n 2 | tr '\n' ' '` >> "${BRANCH}_bench_results.txt"
done
echo

echo Checking out main
git checkout main
make

rm -f "main_bench_results.txt"
for (( i = 0; i < 30; i++ )); do
  ProgressBar $i 30
  echo `./chess2 bench | tail -n 2 | tr '\n' ' '` >> "main_bench_results.txt"
done
echo

echo Checking out $BRANCH
git checkout "$BRANCH"

echo Comparing times
ministat -C 1 -w 50 "main_bench_results.txt" "${BRANCH}_bench_results.txt"
echo Comparing nps
ministat -C 5 -w 50 "main_bench_results.txt" "${BRANCH}_bench_results.txt"
