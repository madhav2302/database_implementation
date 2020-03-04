#!/bin/sh
make a1test.out
for i in {1..8}; do
  printf "1\n${i}\n" | ./a1test.out
done
