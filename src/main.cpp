#include <iostream>
#include "bitboard.h"
#include "movegen.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    init_leapers();
    
    // Create an empty occupancy bitboard (representing all pieces on the board)
    U64 occupancy = 0ULL;
    
    // Place blockers to intercept the Rook's rays
    set_bit(occupancy, d6); // Blocker above the rook
    set_bit(occupancy, f4); // Blocker to the right of the rook
    
    std::cout << "\nOccupancy Board (Blockers on d6 and f4):";
    print_bitboard(occupancy);
    
    // Generate attacks for a Rook placed on d4, passing in our blockers
    U64 rook_mask = get_rook_attacks(d4, occupancy);
    
    std::cout << "\nRook Attacks from d4 (Notice how the rays stop at the blockers):";
    print_bitboard(rook_mask);
    
    // Queen test: Combine Rook and Bishop attacks on the same square
    U64 queen_mask = get_rook_attacks(d4, occupancy) | get_bishop_attacks(d4, occupancy);
    
    std::cout << "\nQueen Attacks from d4 (Rook | Bishop):";
    print_bitboard(queen_mask);
    
    return 0;
}