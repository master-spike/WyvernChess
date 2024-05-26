#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include  <iomanip>

#include "magicbb.h"

namespace Wyvern {

MagicTable::MagicTable()
:knight_table{}
,king_table{}
,bishop_magics{}
,rook_magics{}
,passed_pawns{} {
  initialiseAllMagics(bishop_magics, rook_magics);
  for (int i = 0; i < 64; i++) {
    U64 kf = (1ULL << i);
    if (i >= 8) kf |= kf >> 8;
    if (i < 56) kf |= kf << 8;
    U64 kfr = (kf << 1) & ~(FILE_A);
    U64 kfl = (kf >> 1) & ~(FILE_H);
    king_table[i] = (kf | kfr | kfl) & ~(1ULL << i);

    U64 filem = FILE_A << (i % 8);
    U64 filer = (filem << 1) & ~(FILE_A);
    U64 filel = (filem >> 1) & ~(FILE_H);
    U64 filerr = (filer << 1) & ~(FILE_A);
    U64 filell =  (filel >> 1) & ~(FILE_H);
    U64 ranktt = (i / 8 < 6) ? ranks[i / 8 + 2] : 0;
    U64 rankt = (i / 8 < 7) ? ranks[i / 8 + 1] : 0;
    U64 rankb = (i / 8 > 0) ? ranks[i / 8 - 1] : 0;
    U64 rankbb = (i / 8 > 1) ? ranks[i / 8 - 2] : 0;
    knight_table[i]  = (ranktt | rankbb) & (filer | filel);
    knight_table[i] |= (filell | filerr) & (rankt | rankb);

    U64 pp_files = files[i % 8];
    pp_files |= (pp_files << 1 & ~FILE_A) | (pp_files >> 1 & ~FILE_H);
    U64 pp_white = pp_files;
    U64 pp_black = pp_files;
    for (int j = 0; j <= i/8; j++) {
      pp_white &= ~ranks[j];
    }
    for (int j = 7; j >= i/8; j--) {
      pp_black &= ~ranks[j];
    }
    passed_pawns[i] = pp_white;
    passed_pawns[i + 64] = pp_black;
  }
}

MagicTable::MagicTable(MagicTable &mt) {
  for (int i = 0; i < 64; i++) {
    knight_table[i] = mt.knight_table[i];
    king_table[i] = mt.king_table[i];
    rook_magics[i] = MagicBB(mt.rook_magics[i]);
    bishop_magics[i] = MagicBB(mt.bishop_magics[i]);
    passed_pawns[i] = mt.passed_pawns[i];
    passed_pawns[i+64] = mt.passed_pawns[i+64];
  }
}

MagicBB::MagicBB(int sq, U64 mg, U64 ms, int b){
  square = sq;
  magicnum = mg;
  mask = ms;
  table = std::vector<U64>(1 << b, 0);
  bits = b;
}

U64 MagicBB::compute(U64 blockers) {
  blockers &= mask;
  blockers *= magicnum;
  blockers = blockers >> (64 - bits);
  return table[blockers];
}

constexpr U64 rook_magic_numbers[64] = {
  0x8080008118604002ULL,
  0x4040100040002002ULL,
  0x0080100018e00380ULL,
  0x0100041002200900ULL,
  0x0200020008100420ULL,
  0x4100040002880100ULL,
  0x0080008002000100ULL,
  0x8100014028820300ULL,
  0x0860802080004008ULL,
  0x0112004081020024ULL,
  0x1042002010408200ULL,
  0x00410010000b0020ULL,
  0x0020800800800400ULL,
  0x0004808026000400ULL,
  0x0820800100800200ULL,
  0x0d43000a00a04900ULL,
  0x4080818000400068ULL,
  0x0020818040002005ULL,
  0x00a0010018410020ULL,
  0x8010004008040041ULL,
  0x0028008008800400ULL,
  0x0809010002080400ULL,
  0x1040240048311230ULL,
  0x0088020000d28425ULL,
  0x1480004440002010ULL,
  0x2020400440201000ULL,
  0x2000200080100080ULL,
  0x1400280280300080ULL,
  0x4028002500181100ULL,
  0x8040040080800200ULL,
  0x0800020400108108ULL,
  0x003041120004408cULL,
  0x0080804008800020ULL,
  0x4010002000400040ULL,
  0x2000100080802000ULL,
  0x8300810804801000ULL,
  0x8011001205000800ULL,
  0x0810800601800400ULL,
  0x4301083214000150ULL,
  0x204026458e001401ULL,
  0x0040204000808000ULL,
  0x8001008040010020ULL,
  0x8410820820420010ULL,
  0x1003001000090020ULL,
  0x0804040008008080ULL,
  0x0012000810020004ULL,
  0x1000100200040208ULL,
  0x430000a044020001ULL,
  0x0280009023410300ULL,
  0x00e0100040002240ULL,
  0x0000200100401700ULL,
  0x2244100408008080ULL,
  0x0008000400801980ULL,
  0x0002000810040200ULL,
  0x8010100228810400ULL,
  0x2000009044210200ULL,
  0x4080008040102101ULL,
  0x0040002080411d01ULL,
  0x2005524060000901ULL,
  0x0502001008400422ULL,
  0x489a000810200402ULL,
  0x0001004400080a13ULL,
  0x4000011008020084ULL,
  0x0026002114058042ULL
};

constexpr U64 bishop_magic_numbers[64] = {
  0x0420c80100408202ULL,
  0x1204311202260108ULL,
  0x2008208102030000ULL,
  0x00024081001000caULL,
  0x0488484041002110ULL,
  0x001a080c2c010018ULL,
  0x00020a02a2400084ULL,
  0x0440404400a01000ULL,
  0x0008931041080080ULL,
  0x0000200484108221ULL,
  0x0080460802188000ULL,
  0x4000090401080092ULL,
  0x4000011040a00004ULL,
  0x0020011048040504ULL,
  0x2008008401084000ULL,
  0x000102422a101a02ULL,
  0x2040801082420404ULL,
  0x8104900210440100ULL,
  0x0202101012820109ULL,
  0x0248090401409004ULL,
  0x0044820404a00020ULL,
  0x0040808110100100ULL,
  0x0480a80100882000ULL,
  0x184820208a011010ULL,
  0x0110400206085200ULL,
  0x0001050010104201ULL,
  0x4008480070008010ULL,
  0x8440040018410120ULL,
  0x0041010000104000ULL,
  0x4010004080241000ULL,
  0x0001244082061040ULL,
  0x0051060000288441ULL,
  0x0002215410a05820ULL,
  0x6000941020a0c220ULL,
  0x00f2080100020201ULL,
  0x8010020081180080ULL,
  0x0940012060060080ULL,
  0x0620008284290800ULL,
  0x0008468100140900ULL,
  0x418400aa01802100ULL,
  0x4000882440015002ULL,
  0x0000420220a11081ULL,
  0x0401a26030000804ULL,
  0x0002184208000084ULL,
  0xa430820a0410c201ULL,
  0x0640053805080180ULL,
  0x4a04010a44100601ULL,
  0x0010014901001021ULL,
  0x0422411031300100ULL,
  0x0824222110280000ULL,
  0x8800020a0b340300ULL,
  0x00a8000441109088ULL,
  0x0404000861010208ULL,
  0x0040112002042200ULL,
  0x02141006480b00a0ULL,
  0x2210108081004411ULL,
  0x2010804070100803ULL,
  0x7a0011010090ac31ULL,
  0x0018005100880400ULL,
  0x8010001081084805ULL,
  0x400200021202020aULL,
  0x04100342100a0221ULL,
  0x0404408801010204ULL,
  0x6360041408104012ULL
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
    for (int i = 0; i < 64; ++i) {
      bishops[i] = MagicBB(i, bishop_magic_numbers[i], getPremask<BISHOP>(i), magicBBits[i]);
      if (bishops[i].initialise<BISHOP>()){
        delete[] table;
        std::cout << "Bishop magic initialisation failed" << std::endl;
        return nullptr;
      }
    }
    for (int i = 0; i < 64; ++i) {
      rooks[i] = MagicBB(i, rook_magic_numbers[i], getPremask<ROOK>(i), magicRBits[i]);
      if (rooks[i].initialise<ROOK>()){
        delete[] table;
        std::cout << "Rook magic initialisation failed" << std::endl;
        return nullptr;
      }
    }
    delete[] table;
    return nullptr;
  }

}
