#!/bin/bash
gcc -ansi -Wall *.c -o askish2 -lpthread -lm
./askish2 -D 10 -t 1 -T 3 -lo 2 -hi 3 -S 20 -alg 1

