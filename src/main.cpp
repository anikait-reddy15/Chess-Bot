#include <iostream>
#include "bitboard.h"
#include "movegen.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    
    // Pre-calculate all static attacks at startup
    init_leapers();
    
    // Test White Pawn on e4 (should attack d5 and f5)
    std::cout << "\nWhite Pawn Attacks from e4:";
    print_bitboard(pawn_attacks[white][e4]);
    
    // Test Black Pawn on h7 (should only attack g6, preventing A-file wrap)
    std::cout << "\nBlack Pawn Attacks from h7 (edge case):";
    print_bitboard(pawn_attacks[black][h7]);
    
    return 0;
}
