#include "position.h"
#include "search.h"
#include <iostream>

const char kiwipete_fen[56] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";

int main() {
    Wyvern::Search search;
    Wyvern::Position position(kiwipete_fen);

    int captures = 0;
    int en_passant = 0;
    int castles = 0;
    int promotions = 0;
    int checks = 0;

    const U64 nodes = search.perft(position, 3, &captures, &en_passant, &promotions, &castles, &checks);
    std::cout << "WyvernChess smoke perft: " << nodes << " nodes at kiwipete depth 3\n";
    return 0;
}
