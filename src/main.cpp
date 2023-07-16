#include "movegen.h"
#include "position.h"
#include "search.h"
#include "magicbb.h"
#include <iostream>
#include <iomanip>

int main() {

    for (int i = 0; i < 10; i++) {
        Wyvern::Search my_search;
        Wyvern::Position position;
        int x = my_search.perft(position, i);
        std::cout << "perft " << i << ": " << x << std::endl;
    }
}