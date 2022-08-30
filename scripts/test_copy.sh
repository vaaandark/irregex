#!/bin/bash

cd "$(dirname "$0")" || exit 1
mv ../src/main.c{,.bak}
cp test_copy_main.c ../src/main.c
cd ..
make clean && make
cd build || exit 1

test_copy() {
  ./regex "$1" > /dev/null
  dot -Tsvg relationship.dot -o relationship.svg
  if diff NFA_copy.dot NFA.dot; then
    echo -e '\033[1;032mOK:\033[0m' "$1"
    return 1
  else
    echo -e '\033[1;31mwrong:\033[0m:' "$1"
    return 0
  fi
}

test_all() {
  local total="$#"
  local counter=0
  for item in "$@"; do
    test_copy "$item"
    ((counter+="$?"))
  done
  echo -n "$counter / $total tests "
  if [[ "$counter" -eq "$total" ]]; then
    echo -e '\033[1;032mpassed\033[0m'
  else
    echo 'passed'
  fi
}

test_all 'a' 'abc' '[a-z]' '[[:digit:]]' 'a|b' 'a|(b|(c|(d|(efg))))' 'a*' \
  '(ab)*' '(a|b)*' '((a|b)*|(cd|(ef|g|hij(kl|(m|n)))))[[:digit:]]z|(dfa)'

mv ../src/main.c{.bak,}
cd ..
make clean
