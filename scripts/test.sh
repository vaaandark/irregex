#!/bin/bash

cd "$(dirname "$0")" || exit 1
cd ../build || exit 1

success=0
failure=0

run() {
  regexp="$1"
  string="$2"
  expected="$3"
  result=$(./irregex "$regexp" "$string")
  if [ "$result" = "$expected" ]; then
    success=$((success + 1))
  else
    failure=$((failure + 1))
    echo "FAIL: regex{$regexp} str{$string} expected{$expected} result{$result}"
  fi
}

# run {regex} {str} {expected}
run 'a' 'a' 'a'
run 'abcdefg' 'abcdefg' 'abcdefg'
run '[a-z]*' 'abcdefg' 'abcdefg'
run '(ab|cd)*' 'abcdcdabefg' 'abcdcdab'
run 'a*ax' 'ax' 'ax'
run 'a+|aab*' 'aab' 'aab'
run 'ab*c' 'ac' 'ac'
run 'x(ab)*y' '' ''

echo "$success SUCCESS"
echo "$failure FAILURE"
echo "$(( 100 * success / (success + failure) ))% passed"

