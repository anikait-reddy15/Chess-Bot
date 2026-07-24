#include <iostream>
#include "bitboard.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    
    std::cout << "\nStarting Chessboard Position:\n";
    print_board();
    
    // Example: Move the e2 pawn to e4 and print the updated board
    pop_bit(bitboards[P], e2);
    set_bit(bitboards[P], e4);
    
    std::cout << "\nBoard after e4:\n";
    print_board();
    
    return 0;
}
