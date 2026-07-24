#include <iostream>
#include "bitboard.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    
    // Create an empty bitboard (all zeros)
    U64 pawns = 0ULL;
    
    // Set a bit on square 8 (which is 'a7' in standard mapping)
    set_bit(pawns, 8);
    
    // Set a bit on square 15 (which is 'h7')
    set_bit(pawns, 15);
    
    // Print the visual representation
    print_bitboard(pawns);
    
    return 0;
}
