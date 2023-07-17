#include "movegen.h"
#include "position.h"
#include "search.h"
#include "magicbb.h"
#include <iostream>
#include <iomanip>

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
    
    for (int i = 0; i < 6; i++) {
        Wyvern::Search my_search;
        Wyvern::Position position;
        int x = my_search.perft(position, i, i);
        std::cout << "perft " << i << ": " << x << std::endl;
    }
    

    
    Wyvern::MoveGenerator movegen;
    while(true){
        std::cin.clear();
        U64 blockers;
        if (!(std::cin >> std::hex >> blockers)) continue;
        int sq;
        if (!(std::cin >> std::dec >> sq)) continue;
        std::cout << std::endl << "Square: ";
        printSq(sq);
        std::cout << std::endl;
        std::cout << "Blockers:" << std::endl;
        printbb(blockers);
        std::cout << "Rook attacks:" << std::endl;
        printbb(movegen.rook_magics[sq].compute(blockers));
        std::cout << "Bishop attacks:" << std::endl;
        printbb(movegen.bishop_magics[sq].compute(blockers));
    }
    
}
