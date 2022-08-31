#!/bin/bash

cd "$(dirname "$0")" || exit 1
gcc lexer.c -O2 -o ../build/posixre-lexer
cd ..

hyperfine --warmup 3 './build/lexer ./bench/huge.c' './build/posixre-lexer ./bench/huge.c'

rm ./build/posixre-lexer

