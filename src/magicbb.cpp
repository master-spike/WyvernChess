#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include  <iomanip>

#include "magicbb.h"

namespace Wyvern {


template<enum PieceType PT>
U64 getPremask(int p) {
  if (p >= 64 || p < 0) return 0;
  if constexpr (PT == BISHOP) {
    U64 left = FILE_A << (p & 7);
    U64 right = left;
    U64 up = RANK_1 << (p & 56);
    U64 down = up;
    U64 out = 0;
    for (int i = 1; i < 8; ++i) {
      left >>= 1;
      left &= ~(FILE_H); // prevent wraparound
      right <<= 1;
      right &= ~(FILE_A); // prevent wraparound
      up <<= 8;
      down >>= 8;
      out |= (left | right) & (up | down); 
    }
    return out;
  }
  else if constexpr (PT == ROOK) {
    U64 file = FILE_A << (p & 7);
    U64 rank = RANK_1 << (p & 56);
    return file ^ rank; // xor to ensure 0 at intersection of f and r
  }
  else return 0; // we're only interested in sliders here
}

template<enum PieceType PT>
int MagicBB::initialise() {
  if (PT != ROOK && PT != BISHOP) return 1;
  for (int i = 0; i < (1 << bits); ++i) {
    table[i] = 0;
  }
  U64 blockers = 0;
  for (int i = 0; i < (1 << bits); i++) {
    U64 attacks = generateAttacks<PT>(square, blockers);
    U64 index = (magicnum * blockers) >> (64 - bits);
    if (!table[index]) table[index] = attacks;
    else if (table[index] != attacks) return 1; 
    blockers = (blockers - mask) & mask;
  }
  return 0;
}

template<enum PieceType PT>
U64 generateAttacks(int p, U64 blockers) {
  if constexpr(PT == ROOK) {
    int q = p;
    U64 out = 0;
    // do up:
    while(q < 64) {
      q += 8;
      out |= 1ULL << q;
      if (blockers & (1ULL << q)) break;
    }
    q = p;
    while(q >= 0) {
      q -= 8;
      out |= 1ULL << q;
      if (blockers & (1ULL << q)) break;
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
    int dir = 9;
    U64 filter = FILE_A;
    while (sq) {
      sq <<= dir;
      sq &= ~filter;
      out |= sq;
      sq &= ~blockers;
    }
    sq = 1ULL << p;
    dir = 7;
    while (sq) {
      sq >>= dir;
      sq &= ~filter;
      out |= sq;
      sq &= ~blockers;
    }
    sq = 1ULL << p;
    filter = FILE_H;
    while(sq) {
      sq <<= dir;
      sq &= ~filter;
      out |= sq;
      sq &= ~blockers;
    }
    sq = 1ULL << p;
    dir = 9;
    while(sq) {
      sq >>= dir;
      sq &= ~filter;
      out |= sq;
      sq &= ~blockers;
    }
    return out;
  }
  return 0;
}

template U64 findMagicNum<ROOK>(int p);
template U64 findMagicNum<BISHOP>(int p);


MagicBB::MagicBB(int sq, U64 mg, U64 ms, U64* tb, int b){
  square = sq;
  magicnum = mg;
  mask = ms;
  table = tb;
  bits = b;
}

U64 MagicBB::compute(U64 blockers) {
  blockers &= mask;
  blockers *= magicnum;
  blockers >>= 64 - bits;
  return table[blockers];
}

const U64 rook_magic_numbers[64] = {
  0x4280008040102301ULL,
  0x0008104023005008ULL,
  0x0200104021080000ULL,
  0x0200200810040000ULL,
  0x96000a0212000000ULL,
  0x0200090402086200ULL,
  0x2200440102002518ULL,
  0x0100124220850001ULL,
  0x0021800080400002ULL,
  0x6001400050205000ULL,
  0x0206001448408020ULL,
  0x0312001a20000290ULL,
  0x0202000804080000ULL,
  0x102a000200040e0dULL,
  0x0884004102080000ULL,
  0x0406000900a04800ULL,
  0x8000224000400000ULL,
  0x0040081000230080ULL,
  0x02405200208000c1ULL,
  0x0088120020104092ULL,
  0x0790020006542008ULL,
  0x0200020004040004ULL,
  0x4202840003008084ULL,
  0x0002820005005500ULL,
  0x0468488080020840ULL,
  0x8001004110000000ULL,
  0x8100001008080040ULL,
  0x8000010a00220800ULL,
  0x12000011c0010102ULL,
  0x2000000440010018ULL,
  0x0400000400010002ULL,
  0x4082844a00210001ULL,
  0x0000008468200280ULL,
  0x0000004001002080ULL,
  0x0000008028200005ULL,
  0x0002220172008900ULL,
  0x0000402150020010ULL,
  0x2000046448010800ULL,
  0x0000000100800000ULL,
  0x0200000041c00001ULL,
  0x0000052080012004ULL,
  0x0000114820202062ULL,
  0xc000021001488402ULL,
  0x8000008008008000ULL,
  0x0000002040180800ULL,
  0x0020001004080000ULL,
  0x40000010020c2070ULL,
  0x0200000100428000ULL,
  0x0000005104420c00ULL,
  0x0000040022060020ULL,
  0x0000020018105c84ULL,
  0x00000010000d0100ULL,
  0x3000006008405000ULL,
  0x00000004400a1000ULL,
  0x0000000102840800ULL,
  0x000000220400a800ULL,
  0x0213004000208003ULL,
  0x8100400010002001ULL,
  0x4420010008001001ULL,
  0x8403200840100004ULL,
  0x6040402004888210ULL,
  0x00a9080240142050ULL,
  0x602401280210a010ULL,
  0x105202c5000080b4ULL
};

const U64 bishop_magic_numbers[64] = {
  0x0420c80100408202ULL,
  0x8082088200880000ULL,
  0x0008064412000000ULL,
  0x0004040090020600ULL,
  0x0011104001024222ULL,
  0x0002d00404008409ULL,
  0x8084008888408800ULL,
  0x0002402208124020ULL,
  0x0002081110008010ULL,
  0x0040b80800008245ULL,
  0xa00450080080c1a0ULL,
  0x0000024000002004ULL,
  0x0020441400d91005ULL,
  0x801044810000044aULL,
  0x0008011100000000ULL,
  0x000018d042084810ULL,
  0x004120080a280040ULL,
  0x8004144200000400ULL,
  0x000401e0b0000000ULL,
  0x0004100b40200808ULL,
  0x0001000814000000ULL,
  0x0000704028000000ULL,
  0x0000452280102080ULL,
  0x0192010082008113ULL,
  0xe020120004a41400ULL,
  0x2010140001300004ULL,
  0x1004900008001400ULL,
  0x0002008020100020ULL,
  0x000084000a011000ULL,
  0x0024428008400000ULL,
  0x1003020410502508ULL,
  0x1004012000908000ULL,
  0x1004104012050000ULL,
  0x4001100a00188014ULL,
  0x1000206100404008ULL,
  0x00000400a00b0400ULL,
  0x00100200800a1031ULL,
  0x0084004180804000ULL,
  0x0008011903200000ULL,
  0x0001040080107101ULL,
  0x0001100211080000ULL,
  0x201120d000540000ULL,
  0x4048840088001000ULL,
  0x4308005111000a00ULL,
  0x0000a00200844004ULL,
  0x0088010400200400ULL,
  0x0114310180601401ULL,
  0x0011064900c00304ULL,
  0x4000888152000080ULL,
  0x0010808410804830ULL,
  0x140000aa00980001ULL,
  0x0201000050400000ULL,
  0x1802000c0c400150ULL,
  0x000044a08200000cULL,
  0x8011040184000048ULL,
  0x0030048110500100ULL,
  0x2000460805081008ULL,
  0x6400008090980040ULL,
  0xb000298c84012010ULL,
  0x0600018203090004ULL,
  0x4000200040048801ULL,
  0x0001813002820000ULL,
  0x2000300410008138ULL,
  0x00100481104c8000ULL
};

  U64* initialiseAllMagics(MagicBB* bishops, MagicBB* rooks) {
    size_t table_size = 0;
    size_t table_size_bishop = 0;
    for (int i = 0; i < 64; i++) {
      table_size += (1ULL << magicBBits[i]) + (1ULL << magicRBits[i]);
      table_size_bishop += (1ULL << magicBBits[i]);
    }
    U64* table = new U64[table_size];
    if (!table) return 0;
    U64* iter = table;
    for (int i = 0; i < 64 && iter < table + table_size_bishop; iter += (1ULL << magicBBits[i]), ++i) {
      bishops[i] = MagicBB(i, bishop_magic_numbers[i], getPremask<BISHOP>(i), iter, magicBBits[i]);
      if (bishops[i].initialise<BISHOP>()){
        delete[] table;
        return nullptr;
      }
    }
    for (int i = 0; i < 64 && iter < table + table_size; iter += (1ULL << magicRBits[i]), ++i) {
      rooks[i] = MagicBB(i, rook_magic_numbers[i], getPremask<ROOK>(i), iter, magicRBits[i]);
      if (rooks[i].initialise<ROOK>()){
        delete[] table;
        return nullptr;
      }
    }

    return table;
  }

}
