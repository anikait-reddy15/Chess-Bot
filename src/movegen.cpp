#include "movegen.h"
#include "move.h" // Needed to use the ENCODE_MOVE macro

// Define the global lookup tables
U64 knight_attacks[64];
U64 king_attacks[64];
U64 pawn_attacks[2][64];

U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    attacks |= (bitboard << 17) & not_a_file;
    attacks |= (bitboard << 15) & not_h_file;
    attacks |= (bitboard << 10) & not_ab_file;
    attacks |= (bitboard << 6)  & not_hg_file;
    attacks |= (bitboard >> 17) & not_h_file;
    attacks |= (bitboard >> 15) & not_a_file;
    attacks |= (bitboard >> 10) & not_hg_file;
    attacks |= (bitboard >> 6)  & not_ab_file;
    return attacks;
}

U64 mask_king_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    attacks |= (bitboard >> 8);
    attacks |= (bitboard << 8);
    attacks |= (bitboard << 1) & not_a_file;
    attacks |= (bitboard >> 1) & not_h_file;
    attacks |= (bitboard >> 7) & not_a_file;
    attacks |= (bitboard >> 9) & not_h_file;
    attacks |= (bitboard << 9) & not_a_file;
    attacks |= (bitboard << 7) & not_h_file;
    return attacks;
}

U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    if (side == white) {
        attacks |= (bitboard >> 7) & not_a_file;
        attacks |= (bitboard >> 9) & not_h_file;
    } else {
        attacks |= (bitboard << 7) & not_h_file;
        attacks |= (bitboard << 9) & not_a_file;
    }
    return attacks;
}

U64 get_bishop_attacks(int square, U64 block) {
    U64 attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;
    for (int i = r - 1, j = f + 1; i >= 0 && j <= 7; i--, j++) {
        U64 sq = 1ULL << (i * 8 + j);
        attacks |= sq;
        if (sq & block) break;
    }
    for (int i = r - 1, j = f - 1; i >= 0 && j >= 0; i--, j--) {
        U64 sq = 1ULL << (i * 8 + j);
        attacks |= sq;
        if (sq & block) break;
    }
    for (int i = r + 1, j = f + 1; i <= 7 && j <= 7; i++, j++) {
        U64 sq = 1ULL << (i * 8 + j);
        attacks |= sq;
        if (sq & block) break;
    }
    for (int i = r + 1, j = f - 1; i <= 7 && j >= 0; i++, j--) {
        U64 sq = 1ULL << (i * 8 + j);
        attacks |= sq;
        if (sq & block) break;
    }
    return attacks;
}

U64 get_rook_attacks(int square, U64 block) {
    U64 attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;
    for (int i = r - 1; i >= 0; i--) {
        U64 sq = 1ULL << (i * 8 + f);
        attacks |= sq;
        if (sq & block) break;
    }
    for (int i = r + 1; i <= 7; i++) {
        U64 sq = 1ULL << (i * 8 + f);
        attacks |= sq;
        if (sq & block) break;
    }
    for (int i = f - 1; i >= 0; i--) {
        U64 sq = 1ULL << (r * 8 + i);
        attacks |= sq;
        if (sq & block) break;
    }
    for (int i = f + 1; i <= 7; i++) {
        U64 sq = 1ULL << (r * 8 + i);
        attacks |= sq;
        if (sq & block) break;
    }
    return attacks;
}

void init_leapers() {
    for (int square = 0; square < 64; square++) {
        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    }
}

// This function will loop through all pieces and push their moves to the MoveList
void generate_moves(MoveList &move_list, int side) {
    // Determine the piece IDs based on whose turn it is
    int piece_n = (side == white) ? N : n;
    int piece_k = (side == white) ? K : k;

    // Create a bitboard of all friendly pieces to prevent self-captures (Friendly Fire)
    U64 friendly_occupancy = 0ULL;
    if (side == white) {
        friendly_occupancy = bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K];
    } else {
        friendly_occupancy = bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k];
    }

    // 1. Generate Knight Moves
    U64 knights = bitboards[piece_n]; // Get a copy of the knight bitboard
    while (knights) {
        int source_square = get_lsb_index(knights); // Find the first knight
        
        // Get pre-calculated attacks and mask out friendly pieces using bitwise NOT (~)
        U64 attacks = knight_attacks[source_square] & ~friendly_occupancy; 

        // Loop through all valid attacked squares
        while (attacks) {
            int target_square = get_lsb_index(attacks);
            
            // Encode the move and add it to the list
            int move = ENCODE_MOVE(source_square, target_square, piece_n, 0, 0, 0, 0, 0);
            move_list.add_move(move);
            
            // Delete the processed attack square so we can find the next one
            pop_bit(attacks, target_square);
        }
        
        // Delete the processed knight so we can find the next one
        pop_bit(knights, source_square);
    }

    // 2. Generate King Moves
    U64 kings = bitboards[piece_k];
    while (kings) {
        int source_square = get_lsb_index(kings);
        
        // Get pre-calculated attacks and mask out friendly pieces
        U64 attacks = king_attacks[source_square] & ~friendly_occupancy;

        while (attacks) {
            int target_square = get_lsb_index(attacks);
            int move = ENCODE_MOVE(source_square, target_square, piece_k, 0, 0, 0, 0, 0);
            move_list.add_move(move);
            pop_bit(attacks, target_square);
        }
        pop_bit(kings, source_square);
    }
}