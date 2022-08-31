#!/bin/bash

if [[ $# -ne 1 ]]; then
  echo "usage: draw.sh {regexp}"
  exit 1
fi

cd "$(dirname "$0")" || exit 1
cd ../build/ || exit 1
./re2tree "$1"
./re2graph "$1"
dot -Tsvg RE-tree.dot -o RE-tree.svg
dot -Tsvg NFA-graph.dot -o NFA-graph.svg

