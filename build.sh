#!/bin/bash

set -xe

(
  cd "$(dirname "$0")" || exit 1
  mkdir -p build
  gcc -Werror -Wshadow -g -O0 regex.c -o ./build/regex
#  gcc -Werror -Wshadow -g -fsanitize=address -O0 regex.c -o ./build/regex

#  cd build
#  ./regex > graph.dot
#  dot -Tsvg graph.dot -o graph.svg
)
