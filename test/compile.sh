#!/bin/sh

for file in ./*.c
do
  clang -emit-llvm -S -c $file
done
