#include "bitboard.h"

void print_bitboard(U64 bitboard) {
    std::cout << "\n";
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            // Convert rank and file to a square index (0 to 63)
            int square = rank * 8 + file;
            
            // Print rank numbers on the left side
            if (!file) {
                std::cout << 8 - rank << "  ";
            }
            
            // Print the bit state (1 or 0)
            std::cout << get_bit(bitboard, square) << " ";
        }
        std::cout << "\n";
    }
    
    // Print file letters at the bottom
    std::cout << "\n   a b c d e f g h\n\n";
    
    // Print the decimal value of the bitboard
    std::cout << "   Bitboard: " << bitboard << "\n\n";
}