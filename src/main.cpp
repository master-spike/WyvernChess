#include "movegen.h"
#include "position.h"
#include "search.h"
#include "magicbb.h"
#include "evaluate.h"
#include <iostream>
#include <memory>
#include <iomanip>
#include <ctime>

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
    for (int i = 0; i < 5; i++) {
        Wyvern::Search my_search;
        Wyvern::Position position;
        int n_capts = 0;
        int n_enpass = 0;
        int n_castles = 0;
        int n_promo = 0;
        int checks = 0;
        U64 x = my_search.perft(position, i, &n_capts, &n_enpass, &n_promo, &n_castles, &checks);
        std::cout << std::dec << "perft " << i << ": " << x
                  << ", captures=" << n_capts
                  << ", en-passant=" << n_enpass
                  << ", castles=" << n_castles
                  << ", promotions=" << n_promo
                  << ", checks=" << checks
                  << std::endl;
    }
    for (int i = 0; i < 5; i++) {
        Wyvern::Search my_search;
        Wyvern::Position position(kiwipete_fen);
        int n_capts = 0;
        int n_enpass = 0;
        int n_castles = 0;
        int n_promo = 0;
        int checks = 0;
        U64 x = my_search.perft(position, i, &n_capts, &n_enpass, &n_promo, &n_castles, &checks);
        std::cout << std::dec << "perft kiwipere " << i << ": " << x
                  << ", captures=" << n_capts
                  << ", en-passant=" << n_enpass
                  << ", castles=" << n_castles
                  << ", promotions=" << n_promo
                  << ", checks=" << checks
                  << std::endl;
    }
    {
        std::shared_ptr<Wyvern::MagicTable> mt = std::make_shared<Wyvern::MagicTable>();
        Wyvern::MoveGenerator mgen(mt);
        Wyvern::Evaluator my_evaluator(mt);
        Wyvern::Position position(kiwipete_fen);
        position.printPretty();
        std::vector<U32> moves; mgen.generateMoves<Wyvern::COLOR_WHITE>(position, true, &moves);
        for (U32 move : moves) {
            if (move & Wyvern::YES_CAPTURE) {
                int seeval = my_evaluator.seeCapture<Wyvern::COLOR_WHITE>(position, move);
                std::cout << "See value of ";
                printSq(move & 63); printSq((move >> 6) & 63);
                std::cout << " == "  << seeval << "\n";
            } 
        }
    }


    std::cin.get();

    Wyvern::Position pos;

    std::shared_ptr<Wyvern::MagicTable> tt_ptr = std::make_shared<Wyvern::MagicTable>();

    Wyvern::Evaluator evaluator(tt_ptr);

    while (true)
    {
        time_t t0 = time(nullptr);
        Wyvern::Search my_search;
        pos.printPretty();
        int result;
        U32 chosen_move = my_search.bestmove(pos, 15, 7, 14, result);
        std::cout << std::dec << "Static evaluation "
                  << (-2 * pos.getToMove() + 1) * evaluator.evalPositional(pos)
                  << " | Dynamic evaluation " << (-2 * pos.getToMove() + 1) * result
                  << std::endl << std::endl;
        if (chosen_move == Wyvern::MOVE_NONE) break;
        pos.makeMove(chosen_move);
        while(difftime(time(nullptr), t0) < 0.5) {

        }
    }
    pos.printPretty();
    
    
}
