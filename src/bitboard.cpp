#include "bitboard.h"

#include "bitboard.h"

// Initialize the 12 bitboards with the standard chess starting position
U64 bitboards[12] = {
    0x00FF000000000000ULL, // P (White Pawns)
    0x4200000000000000ULL, // N (White Knights)
    0x2400000000000000ULL, // B (White Bishops)
    0x8100000000000000ULL, // R (White Rooks)
    0x0800000000000000ULL, // Q (White Queen)
    0x1000000000000000ULL, // K (White King)
    0x000000000000FF00ULL, // p (Black Pawns)
    0x0000000000000042ULL, // n (Black Knights)
    0x0000000000000024ULL, // b (Black Bishops)
    0x0000000000000081ULL, // r (Black Rooks)
    0x0000000000000008ULL, // q (Black Queen)
    0x0000000000000010ULL  // k (Black King)
};

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