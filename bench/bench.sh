#!/bin/bash

cd "$(dirname "$0")" || exit 1
gcc lexer.c -O2 -o ../build/posixre-lex
cd ..

hyperfine --warmup 3 './build/lex ./bench/huge.c' './build/posixre-lex ./bench/huge.c'

rm ./build/posixre-lex

