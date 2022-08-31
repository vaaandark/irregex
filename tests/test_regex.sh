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

# email
run '[a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\.[a-zA-Z0-9_-]+)+' \
  'vaaandark@outlook.com' 'vaaandark@outlook.com'

# mobile phone number
run '1(3|4|5|6|7|8|9)\d{9}' '13800000000' '13800000000'

# domain name
run '((http:\/\/)|(https:\/\/))?([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,6}(\/)' \
  'https://google.com/' 'https://google.com/'
run '((http:\/\/)|(https:\/\/))?([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,6}(\/)' \
  'https://google.com' ''

run '[1-9]\d*\.\d*|0\.\d*[1-9]\d*' '3.14' '3.14'
run '[1-9]\d*\.\d*|0\.\d*[1-9]\d*' '-3.14' ''
run '[1-9]\d*\.\d*|0\.\d*[1-9]\d*' '3' ''

echo "$success SUCCESS"
echo "$failure FAILURE"
echo "$(( 100 * success / (success + failure) ))% passed"

