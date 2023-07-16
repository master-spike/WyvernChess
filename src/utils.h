#ifndef H_GUARD_UTILS
#define H_GUARD_UTILS

#include "types.h"
#include <ctime>
#include <cstdlib>
#include <iostream>

#define MAX_INT(x,y) (x > y) ? x : y;

U64 rand64();

void seedRand();

int popCount64(U64 v);

void printbb(U64 bb);

void printSq(int p);

#endif