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
    U64 left = 0x0101010101010101ULL << (p % 8);
    U64 right = left;
    U64 up = 0xFF << ((p / 8)*8);
    U64 down = up;
    U64 out = 0;
    for (int i = 1; i < 8; ++i) {
      left >>= 1;
      left &= ~(0x8080808080808080ULL); // prevent wraparound
      right <<= 1;
      right &= ~(0x0101010101010101ULL); // prevent wraparound
      up <<= 8;
      down >>= 8;
      out |= (left | right) & (up | down); 
    }
    return out;
  }
  else if constexpr (PT == ROOK) {
    U64 file = 0x0101010101010101ULL << (p%8);
    U64 rank = 0xFF << ((p/8)*8);
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
    q = p;
    while(q < ((p / 8 + 1)*8)) {
      q += 1;
      out |= 1ULL << q;
      if (blockers & (1ULL << q)) break;
    }
    q = p;
    while(q >= ((p / 8)*8)) {
      q -= 1;
      out |= 1ULL << q;
      if (blockers & (1ULL << q)) break;
    }
    return out;
  }
  if constexpr(PT == BISHOP) {
    U64 sq = 1ULL << p;
    U64 out = 0;
    int dir = 9;
    U64 filter = 0x0101010101010101ULL;
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
    filter = 0x8080808080808080ULL;
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

template<enum PieceType PT>
U64 findMagicNum(int p) {
  if constexpr(PT != ROOK && PT != BISHOP) return 0;
  int bits = (PT == BISHOP) ? magicBBits[p] : magicRBits[p];
  U64 mask = getPremask<PT>(p);
  U64* temp_table = (U64*) malloc(sizeof(U64) *(size_t) (1ULL << bits));
  for (U64 k = 0; k < MAGIC_MAX_TRIALS; ++k) {
    U64 magic = rand64() & rand64() & rand64() ;
    if (popCount64((magic*mask) & 0xFF00000000000000ULL) < 6) continue;
    MagicBB mbb(p , magic, mask, temp_table, bits);
    if (!mbb.initialise<PT>()) {
      free(temp_table);
      std::cout << "magic number " << p << " for " << ((PT == ROOK) ? "  rook" : "bishop") << " found in " << k+1 << " tries" << "\n";
      return magic;
    }
  }
  std::cout << "Failed to find magic number " << p << " for " << ((PT == ROOK) ? "  rook" : "bishop") << "\n";
  free(temp_table);
  return 0;
}

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

const U64 rook_magics[64] = {
  0x2480016b10804008ULL,
  0x030010462100000aULL,
  0x0100112049000181ULL,
  0x0500090004104020ULL,
  0x45000802050000b1ULL,
  0x2100150004024422ULL,
  0x4500028a01002620ULL,
  0x8200048c29020040ULL,
  0x0085800324402000ULL,
  0x1301002440000008ULL,
  0x0101002210c00004ULL,
  0x070100081020aaaeULL,
  0x010100241801c000ULL,
  0x0101000204000000ULL,
  0x0084008806005802ULL,
  0x4062000040810000ULL,
  0x8090408000200000ULL,
  0x20004240003140d0ULL,
  0x200050100000030cULL,
  0x1000a80800020000ULL,
  0x48002c0400064200ULL,
  0x0100050022000000ULL,
  0x8100030002010200ULL,
  0x02904200010aa000ULL,
  0x4000200420000080ULL,
  0x0000020812000004ULL,
  0x0000008120000120ULL,
  0x0000023100003000ULL,
  0x000000514000000cULL,
  0xa000001c04420009ULL,
  0x0000020100000000ULL,
  0x0004000c02000008ULL,
  0x2492802c10172800ULL,
  0x0004c00810002000ULL,
  0x080000a420020002ULL,
  0x0000401800000104ULL,
  0x0000401008602510ULL,
  0x0001482080000004ULL,
  0x02004004c2823c40ULL,
  0x0000008202921050ULL,
  0x5100002006000082ULL,
  0x8100081080200020ULL,
  0x1020012400020020ULL,
  0x0000085000010024ULL,
  0x0000120401550088ULL,
  0x0080000830808000ULL,
  0x2000001a10841118ULL,
  0x3100002052040000ULL,
  0x000000a004841600ULL,
  0x4100001800000801ULL,
  0x00001000c01080c0ULL,
  0x0004001044008100ULL,
  0x8000204009000001ULL,
  0x0c00000704082020ULL,
  0x000180004988800aULL,
  0x0470000509000000ULL,
  0x2020010000004400ULL,
  0x0808004000268228ULL,
  0x220850400002b808ULL,
  0x21406020000000c0ULL,
  0x0816232000000000ULL,
  0x0804262800010040ULL,
  0x01020824000a9015ULL,
  0x4241004800090012ULL
};

const U64 bishop_magics[64] = {
  0x4804200204002884ULL,
  0x10060800c444a180ULL,
  0x020800cc00201090ULL,
  0x0212208010400808ULL,
  0x0081104100000011ULL,
  0x0010882000018408ULL,
  0x105202b004000241ULL,
  0x040a020201092802ULL,
  0x0000060818080090ULL,
  0x00088a3084480000ULL,
  0x400024000c004100ULL,
  0xc002080908080040ULL,
  0x2002012000810040ULL,
  0x4400109000000024ULL,
  0x2000023c41004205ULL,
  0x1000304865100000ULL,
  0x0040243003020000ULL,
  0x0018000200001340ULL,
  0x0010010a01020a00ULL,
  0x0002010120004020ULL,
  0x000240040300c420ULL,
  0xa020200208422488ULL,
  0x0044000042440200ULL,
  0x0003000200c14020ULL,
  0x0608000000100118ULL,
  0x0201200480000010ULL,
  0x0202000004580004ULL,
  0x0094002100800328ULL,
  0x1001000000012044ULL,
  0x0025894008000000ULL,
  0x0605208600923000ULL,
  0x02300b0100000040ULL,
  0x0000480008821240ULL,
  0x0000120002440008ULL,
  0xc04800210029040aULL,
  0x0800028201006000ULL,
  0x0122020860400a00ULL,
  0x0c542201000048b0ULL,
  0x0000188004820a00ULL,
  0x5c05028601000100ULL,
  0x0000202510400404ULL,
  0x0440003010020400ULL,
  0x0800406088080402ULL,
  0x0000000420048000ULL,
  0x1440000a00080024ULL,
  0x1410001040808801ULL,
  0x0204000088100210ULL,
  0x0090000490000000ULL,
  0x00000000084200c2ULL,
  0x088426004410000cULL,
  0xc000000030801202ULL,
  0x4200100008002000ULL,
  0x0100984000011041ULL,
  0x0042441400400005ULL,
  0xc008088200110184ULL,
  0x0800488003040800ULL,
  0x6000200400110301ULL,
  0x00ca012145080118ULL,
  0x2400400000000050ULL,
  0x0000018042002018ULL,
  0x4000630000344004ULL,
  0x40010600940a8200ULL,
  0x0000104000000008ULL,
  0x4050003040000000ULL
};

}