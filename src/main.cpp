#include <iostream>
#include "bitboard.h"
#include "movegen.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    
    // Pre-calculate all static attacks at startup
    init_leapers();
    
    // Test the O(1) lookup table: Find all squares a Knight attacks from e4
    std::cout << "\nKnight Attacks from e4:";
    print_bitboard(knight_attacks[e4]);
    
    // Test the King lookup table: Find all squares a King attacks from a8 (corner)
    std::cout << "\nKing Attacks from a8:";
    print_bitboard(king_attacks[a8]);
    
    return 0;
}
