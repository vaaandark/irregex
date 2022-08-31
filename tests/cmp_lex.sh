#!/bin/bash

cd "$(dirname "$0")" || exit 1
gcc ../bench/lexer.c -O2 -o ../build/posixre-lex
cd ../src || exit 1

success=0
failure=0

run() {
  code="$1"
  if diff -u <(../build/lex "$code") <(../build/posixre-lex "$code"); then
    ((success++))
  else
    ((failure++))
    echo "FAIL: $code"
  fi
}

for item in *.c; do
  run "$item"
done

for item in *.h; do
  run "$item"
done

echo "$success SUCCESS"
echo "$failure FAILURE"
echo "$(( 100 * success / (success + failure) ))% passed"

rm ../build/posixre-lex

