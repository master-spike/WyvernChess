#include "zobrist.h"

namespace Wyvern {


void randomiseZobristNumbers(U64* ptr, int count, std::string name) {
    if (!(name.empty())){
        std::cout << "const U64" << name << "[" << std::dec << count << "] = {\n"; 
    }
    for (int i = 0; i < count; i++) {
        ptr[i] = rand64();
        std::string sep = (i < count-1 ) ? "ULL," : "ULL";
        if (!name.empty()) {
            std::cout << "  0x" << std::hex << std::setw(16) << std::setfill('0')
                      << ptr[i] << sep << "\n";
        }
    }
    if (!name.empty()) std::cout << "}" << std::endl << std::endl;
}

}