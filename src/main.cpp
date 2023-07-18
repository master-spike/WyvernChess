#include "movegen.h"
#include "position.h"
#include "search.h"
#include "magicbb.h"
#include <iostream>
#include <iomanip>

char kiwipete_fen[56] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";

int main() {
    /*
    for (int i = 0; i < 64; i++) {
        std::cout << "  0x" << std::hex << std::setw(16) << std::setfill('0')
                  << Wyvern::findMagicNum<Wyvern::BISHOP>(i) << "ULL,"<< std::dec << std::endl;
    }
    std::cout << std::endl;
    for (int i = 0; i < 64; i++) {
        std::cout << "  0x" << std::hex << std::setw(16) << std::setfill('0')
                  << Wyvern::findMagicNum<Wyvern::ROOK>(i) << "ULL,"<< std::dec << std::endl;
    }
    
    */
    for (int i = 0; i < 7; i++) {
        Wyvern::Search my_search;
        Wyvern::Position position;
        int n_capts = 0;
        int n_enpass = 0;
        int n_castles = 0;
        int n_promo = 0;
        int checks = 0;
        int x = my_search.perft(position, i, i, &n_capts, &n_enpass, &n_promo, &n_castles, &checks);
        std::cout << std::dec << "perft " << i << ": " << x
                  << ", captures=" << n_capts
                  << ", en-passant=" << n_enpass
                  << ", castles=" << n_castles
                  << ", promotions=" << n_promo
                  << ", checks=" << checks
                  << std::endl;
    }
    for (int i = 0; i < 7; i++) {
        Wyvern::Search my_search;
        Wyvern::Position position(kiwipete_fen);
        int n_capts = 0;
        int n_enpass = 0;
        int n_castles = 0;
        int n_promo = 0;
        int checks = 0;
        int x = my_search.perft(position, i, i, &n_capts, &n_enpass, &n_promo, &n_castles, &checks);
        std::cout << std::dec << "perft kiwipere " << i << ": " << x
                  << ", captures=" << n_capts
                  << ", en-passant=" << n_enpass
                  << ", castles=" << n_castles
                  << ", promotions=" << n_promo
                  << ", checks=" << checks
                  << std::endl;
    }
    
}
