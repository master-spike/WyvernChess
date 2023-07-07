#ifndef H_GUARD_UTILS
#define H_GUARD_UTILS

#include "types.h"
#include <ctime>
#include <cstdlib>

#define MAX_INT(x,y) (x > y) ? x : y;

U64 rand64() {
  U64 u0 = std::rand() & 0xFFFFULL;
  U64 u1 = std::rand() & 0xFFFFULL;
  U64 u2 = std::rand() & 0xFFFFULL;
  U64 u3 = std::rand() & 0xFFFFULL;
  return u0 + (u1 << 16) + (u2 << 32) + (u3 << 48);
}

void seedRand(U32 seed) {
  std::srand(seed);
}

void seedRand() {
    std::srand(std::time(nullptr));
}

inline int popCount64(U64 v) {
    return __builtin_popcountll(v);
}



#endif