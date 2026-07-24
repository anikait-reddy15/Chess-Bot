#include "movegen.h"

// Define the global lookup tables
U64 knight_attacks[64];
U64 king_attacks[64];
U64 pawn_attacks[2][64];

U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    
    // Place the knight on the given square
    set_bit(bitboard, square);

    // Shift in all 8 possible knight directions and mask illegal wrap-arounds
    attacks |= (bitboard << 17) & not_a_file;   // Down 2, Right 1
    attacks |= (bitboard << 15) & not_h_file;   // Down 2, Left 1
    attacks |= (bitboard << 10) & not_ab_file;  // Down 1, Right 2
    attacks |= (bitboard << 6)  & not_hg_file;  // Down 1, Left 2
    
    attacks |= (bitboard >> 17) & not_h_file;   // Up 2, Left 1
    attacks |= (bitboard >> 15) & not_a_file;   // Up 2, Right 1
    attacks |= (bitboard >> 10) & not_hg_file;  // Up 1, Left 2
    attacks |= (bitboard >> 6)  & not_ab_file;  // Up 1, Right 2

    return attacks;
}

U64 mask_king_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    
    // Place the king on the given square
    set_bit(bitboard, square);

    // Shift in all 8 possible king directions and mask illegal wrap-arounds
    attacks |= (bitboard >> 8);                 // Up
    attacks |= (bitboard << 8);                 // Down
    attacks |= (bitboard << 1) & not_a_file;    // Right
    attacks |= (bitboard >> 1) & not_h_file;    // Left
    attacks |= (bitboard >> 7) & not_a_file;    // Up, Right
    attacks |= (bitboard >> 9) & not_h_file;    // Up, Left
    attacks |= (bitboard << 9) & not_a_file;    // Down, Right
    attacks |= (bitboard << 7) & not_h_file;    // Down, Left

    return attacks;
}
U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    
    // Place the pawn on the given square
    set_bit(bitboard, square);

    // Generate White pawn attacks
    if (side == white) {
        // Shift right (Up-Right) and mask the A-file wrap
        attacks |= (bitboard >> 7) & not_a_file;
        
        // Shift left (Up-Left) and mask the H-file wrap
        attacks |= (bitboard >> 9) & not_h_file;
    }
    // Generate Black pawn attacks
    else {
        // Shift right (Down-Left) and mask the H-file wrap
        attacks |= (bitboard << 7) & not_h_file;
        
        // Shift left (Down-Right) and mask the A-file wrap
        attacks |= (bitboard << 9) & not_a_file;
    }

    return attacks;
}

void init_leapers() {
    for (int square = 0; square < 64; square++) {
        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
        
        // Pre-calculate pawn attacks for both colors
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    }
}