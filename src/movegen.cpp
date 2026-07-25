#include "movegen.h"
#include "move.h" 

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

// Core move generation function
void generate_moves(MoveList &move_list, int side) {
    int piece_n = (side == white) ? N : n;
    int piece_k = (side == white) ? K : k;
    int piece_p = (side == white) ? P : p;

    // Build the Occupancy Bitboards
    U64 friendly_occupancy = 0ULL;
    U64 enemy_occupancy = 0ULL;
    if (side == white) {
        friendly_occupancy = bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K];
        enemy_occupancy = bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k];
    } else {
        friendly_occupancy = bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k];
        enemy_occupancy = bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K];
    }
    
    // Total occupancy determines what squares are physically blocked
    U64 occupancy = friendly_occupancy | enemy_occupancy;

    // 1. Generate Knight Moves
    U64 knights = bitboards[piece_n];
    while (knights) {
        int source_square = get_lsb_index(knights);
        U64 attacks = knight_attacks[source_square] & ~friendly_occupancy; 

        while (attacks) {
            int target_square = get_lsb_index(attacks);
            // Check if this move is a capture by testing the target square against enemy occupancy
            int capture = get_bit(enemy_occupancy, target_square) ? 1 : 0;
            move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_n, 0, capture, 0, 0, 0));
            pop_bit(attacks, target_square);
        }
        pop_bit(knights, source_square);
    }

    // 2. Generate King Moves
    U64 kings = bitboards[piece_k];
    while (kings) {
        int source_square = get_lsb_index(kings);
        U64 attacks = king_attacks[source_square] & ~friendly_occupancy;

        while (attacks) {
            int target_square = get_lsb_index(attacks);
            int capture = get_bit(enemy_occupancy, target_square) ? 1 : 0;
            move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_k, 0, capture, 0, 0, 0));
            pop_bit(attacks, target_square);
        }
        pop_bit(kings, source_square);
    }

    // 3. Generate Pawn Moves
    U64 pawns = bitboards[piece_p];
    while (pawns) {
        int source_square = get_lsb_index(pawns);
        
        // PAWN PUSHES (Forward Moves)
        if (side == white) {
            int target_square = source_square - 8; // Move up one rank
            
            // If the square in front is completely empty
            if (target_square >= 0 && !get_bit(occupancy, target_square)) {
                // If on Rank 7, moving forward promotes to a new piece
                if (source_square >= a7 && source_square <= h7) {
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, Q, 0, 0, 0, 0));
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, R, 0, 0, 0, 0));
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, B, 0, 0, 0, 0));
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, N, 0, 0, 0, 0));
                } else {
                    // Standard Single Push
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, 0, 0, 0, 0, 0));
                    
                    // Double Push (Only valid if starting on Rank 2)
                    if (source_square >= a2 && source_square <= h2) {
                        int double_target = target_square - 8;
                        if (!get_bit(occupancy, double_target)) {
                            // Notice we set the 'double_push' flag to 1 here
                            move_list.add_move(ENCODE_MOVE(source_square, double_target, piece_p, 0, 0, 1, 0, 0));
                        }
                    }
                }
            }
        } else { // Black Pawns
            int target_square = source_square + 8; // Move down one rank
            
            if (target_square <= 63 && !get_bit(occupancy, target_square)) {
                // If on Rank 2, moving forward promotes
                if (source_square >= a2 && source_square <= h2) {
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, q, 0, 0, 0, 0));
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, r, 0, 0, 0, 0));
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, b, 0, 0, 0, 0));
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, n, 0, 0, 0, 0));
                } else {
                    move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, 0, 0, 0, 0, 0));
                    
                    // Double Push (Only valid if starting on Rank 7)
                    if (source_square >= a7 && source_square <= h7) {
                        int double_target = target_square + 8;
                        if (!get_bit(occupancy, double_target)) {
                            move_list.add_move(ENCODE_MOVE(source_square, double_target, piece_p, 0, 0, 1, 0, 0));
                        }
                    }
                }
            }
        }

        // PAWN CAPTURES
        // We use our pre-calculated attack masks but strictly limit them to squares containing enemy pieces
        U64 attacks = pawn_attacks[side][source_square] & enemy_occupancy;
        while (attacks) {
            int target_square = get_lsb_index(attacks);
            
            // Promotion Capture check
            if ((side == white && source_square >= a7 && source_square <= h7) || 
                (side == black && source_square >= a2 && source_square <= h2)) {
                int prom_Q = (side == white) ? Q : q;
                int prom_R = (side == white) ? R : r;
                int prom_B = (side == white) ? B : b;
                int prom_N = (side == white) ? N : n;
                
                // Notice capture flag is 1
                move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, prom_Q, 1, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, prom_R, 1, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, prom_B, 1, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, prom_N, 1, 0, 0, 0));
            } else {
                // Standard Capture
                move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_p, 0, 1, 0, 0, 0));
            }
            
            pop_bit(attacks, target_square);
        }
        
        pop_bit(pawns, source_square);
    }

    // 4. Generate Bishop Moves-
    int piece_b = (side == white) ? B : b;
    U64 bishops = bitboards[piece_b];
    while (bishops) {
        int source_square = get_lsb_index(bishops);
        
        // Use our dynamic ray casting, then remove friendly pieces from the targets
        U64 attacks = get_bishop_attacks(source_square, occupancy) & ~friendly_occupancy; 

        while (attacks) {
            int target_square = get_lsb_index(attacks);
            int capture = get_bit(enemy_occupancy, target_square) ? 1 : 0;
            move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_b, 0, capture, 0, 0, 0));
            pop_bit(attacks, target_square);
        }
        pop_bit(bishops, source_square);
    }

    // 5. Generate Rook Moves
    int piece_r = (side == white) ? R : r;
    U64 rooks = bitboards[piece_r];
    while (rooks) {
        int source_square = get_lsb_index(rooks);
        
        U64 attacks = get_rook_attacks(source_square, occupancy) & ~friendly_occupancy; 

        while (attacks) {
            int target_square = get_lsb_index(attacks);
            int capture = get_bit(enemy_occupancy, target_square) ? 1 : 0;
            move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_r, 0, capture, 0, 0, 0));
            pop_bit(attacks, target_square);
        }
        pop_bit(rooks, source_square);
    }

    // 6. Generate Queen Moves
    int piece_q = (side == white) ? Q : q;
    U64 queens = bitboards[piece_q];
    while (queens) {
        int source_square = get_lsb_index(queens);
        
        // Queen attacks are simply a Rook attack and a Bishop attack combined (OR'd)
        U64 attacks = (get_rook_attacks(source_square, occupancy) | get_bishop_attacks(source_square, occupancy)) & ~friendly_occupancy; 

        while (attacks) {
            int target_square = get_lsb_index(attacks);
            int capture = get_bit(enemy_occupancy, target_square) ? 1 : 0;
            move_list.add_move(ENCODE_MOVE(source_square, target_square, piece_q, 0, capture, 0, 0, 0));
            pop_bit(attacks, target_square);
        }
        pop_bit(queens, source_square);
    }
}