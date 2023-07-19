#include "zobrist.h"

int main() {
    U64 zs[64];
    std::vector<std::string> piece_name_list({"pawn","knight","bishop","rook","queen","king"});
    std::vector<std::string> color_name_list({"white","black"});
    for (std::string colorname : color_name_list) {
        for (std::string piecename : piece_name_list) {
            Wyvern::randomiseZobristNumbers(zs, 64,  "zobrist_" + colorname + "_" + piecename);
        }

    }
    Wyvern::randomiseZobristNumbers(zs, 1, "zobrist_tomove");
    Wyvern::randomiseZobristNumbers(zs, 4, "zobrist_castling_rights");
    Wyvern::randomiseZobristNumbers(zs, 8, "zobrist_epfiles");
    return 0;
}
