#!/bin/sh

set -xe

gcc -o invaders  invaders.c `sdl2-config --libs --cflags` -std=c99 -Wall -lm  && ./invaders