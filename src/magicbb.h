#ifndef H_GUARD_MAGIC
#define H_GUARD_MAGIC

#include "types.h"
#include "utils.h"

#define MAGIC_MAX_TRIALS 0x10000000000ULL

namespace Wyvern 
{

template<enum PieceType PT>
U64 generateAttacks(int p, U64 blockers) {
  if constexpr(PT == ROOK) {
    int q = p+8;
    U64 out = 0;
    // do up:
    while(q < 64) {
      out |= 1ULL << q;
      if (blockers & (1ULL << q)) break;
      q += 8;
    }
    q = p-8;
    while(q >= 0) {
      out |= 1ULL << q;
      if (blockers & (1ULL << q)) break;
      q -= 8;
    }
    q = p+1;
    // do right
    while((1ULL << q) & ~(FILE_A)) {
      out |= 1ULL << q;
      if (blockers & (1ULL << q)) break;
      q++;
    }
    q = p-1;
    while((1ULL << q) & ~(FILE_H)) {
      out |= 1ULL << q;
      if (blockers & (1ULL << q)) break;
      q -= 1;
    }
    return out;
  }
  if constexpr(PT == BISHOP) {
    U64 sq = 1ULL << p;
    U64 out = 0;
    while (sq) {
      sq = sq << 9;
      sq &= ~((U64) FILE_A | (U64) RANK_1);
      out |= sq;
      sq &= ~blockers;
    }
    sq = 1ULL << p;
    while (sq) {
      sq = sq >> 7;
      sq &= ~((U64) FILE_A | (U64) RANK_8);
      out |= sq;
      sq &= ~blockers;
    }
    sq = 1ULL << p;
    while(sq) {
      sq = sq << 7;
      sq &= ~((U64) FILE_H | (U64) RANK_1);
      out |= sq;
      sq &= ~blockers;
    }
    sq = 1ULL << p;
    while(sq) {
      sq = sq >> 9;
      sq &= ~((U64) FILE_H | (U64) RANK_8);
      out |= sq;
      sq &= ~blockers;
    }
    return out;
  }
  return 0;
}

class MagicBB {
private:
  int square;
  U64 mask;
  U64 magicnum;
  std::vector<U64> table;
  int bits;
public:
  MagicBB() = default;
  MagicBB(int sq, U64 mg, U64 ms, int b);
  MagicBB(const MagicBB& in_mbb);
  U64 compute(U64 blockers);
  ~MagicBB() = default;
    
  template<enum PieceType PT>
  int initialise() {
    if (PT != ROOK && PT != BISHOP) return 1;
    //printSq(square); std::cout << std::endl;
    //printbb(mask);
    table = std::vector<U64>(1 << bits, 0);
    U64 blockers = 0;
    for (U64 i = 0; i < (1ULL << bits); i++) {
      U64 attacks = generateAttacks<PT>(square, blockers);
      U64 index = (magicnum * blockers) >> (64 - bits);
      //printbb(blockers); std::cout << std::endl; printbb(attacks); std::cout << std::endl;
      //std::cout << "----------------" << std::endl;
      if (!table[index]) table[index] = attacks;
      else if (table[index] != attacks) return 1; 
      blockers = (blockers - mask) & mask;
    }
    //std::cout << std::endl << "================" << std::endl;
    return 0;
  }// returns 1 on failure
};

static const int magicRBits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12};

static const int magicBBits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

template<enum PieceType PT>
U64 getPremask(int p) {
  if (p >= 64 || p < 0) return 0;
  U64 out = 0;
  if constexpr (PT == BISHOP) {
    U64 left = FILE_A << (p & 7);
    U64 right = left;
    U64 up = RANK_1 << (p & 56);
    U64 down = up;

    for (int i = 1; i < 8; ++i) {
      left >>= 1;
      left &= ~(FILE_H); // prevent wraparound
      right <<= 1;
      right &= ~(FILE_A); // prevent wraparound
      up <<= 8;
      down >>= 8;
      out |= (left | right) & (up | down); 
    }
    out &= ~(FILE_A | FILE_H | RANK_1 | RANK_8);
  }
  else if constexpr (PT == ROOK) {
    U64 file = ~(RANK_1 | RANK_8) & (FILE_A << (p & 7));
    U64 rank = ~(FILE_A | FILE_H) & (RANK_1 << (p & 56));
    out = (file | rank) & ~(1ULL << p);
  }
  return out; // we're only interested in sliders here
}



U64* initialiseAllMagics(MagicBB* bishops, MagicBB* rooks);

template<enum PieceType PT>
U64 findMagicNum(int p) {
  if constexpr(PT != ROOK && PT != BISHOP) return 0;
  int bits = (PT == BISHOP) ? magicBBits[p] : magicRBits[p];
  U64 mask = getPremask<PT>(p);
  for (U64 k = 0; k < MAGIC_MAX_TRIALS; ++k) {
    U64 magic = rand64() & rand64() & rand64() ;
    if (popCount64((magic*mask) & 0xFF00000000000000ULL) < 6) continue;
    MagicBB mbb = MagicBB(p, magic, mask, bits);
    if (!mbb.initialise<PT>()) {
      //std::cout << "magic number " << p << " for " << ((PT == ROOK) ? "  rook" : "bishop") << " found in " << k+1 << " tries" << "\n";
      return magic;
    }
  }
  std::cout << "Failed to find magic number " << p << " for " << ((PT == ROOK) ? "  rook" : "bishop") << "\n";
  return 0;
}

template U64 findMagicNum<ROOK>(int p);
template U64 findMagicNum<BISHOP>(int p);
  


}


#endif
