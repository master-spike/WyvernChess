#include "utils.h"

U64 rand64() {
  U64 u0 = std::rand() & 0xFFFFULL;
  U64 u1 = std::rand() & 0xFFFFULL;
  U64 u2 = std::rand() & 0xFFFFULL;
  U64 u3 = std::rand() & 0xFFFFULL;
  return u0 + (u1 << 16) + (u2 << 32) + (u3 << 48);
}

void seedRand() {
    std::srand(std::time(nullptr));
}

int popCount64(U64 v) {
    return __builtin_popcountll(v);
}

void printbb(U64 bb) {
  for (int i = 7; i >= 0; --i) {
    for (int j = 0; j < 8; j++) std::cout << (char)(((bb >> (i*8+j)) & 1) + '0') << " ";
    std::cout << std::endl;
  }

}
