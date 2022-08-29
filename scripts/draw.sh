#!/bin/bash

if [[ $# -ne 1 ]]; then
  echo "usage: test.sh {regexp}"
  exit 1
fi

regexp=$1

(
  cd "$(dirname "$0")" || exit 1
  cd ../build/ || exit 1
  ./regex "$regexp"
  dot -Tsvg relationship.dot -o relationship.svg
  dot -Tsvg NFA.dot -o NFA.svg
)
