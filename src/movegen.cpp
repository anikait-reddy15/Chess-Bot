#include "movegen.h"
#include "move.h"
#include "movelist.h"

// Define the global lookup tables
U64 knight_attacks[64];
U64 king_attacks[64];
U64 pawn_attacks[2][64];

// Castling rights update table. If a piece moves from or to one of these squares, we bitwise AND the castling state
const int castling_rights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

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

int is_square_attacked(int square, int attacking_side) {
    U64 total_occupancy = 0ULL;
    for (int i = P; i <= k; i++) total_occupancy |= bitboards[i];

    // 1. Attacked by pawns
    if (attacking_side == white) {
        if (pawn_attacks[black][square] & bitboards[P]) return 1;
    } else {
        if (pawn_attacks[white][square] & bitboards[p]) return 1;
    }

    // 2. Attacked by knights
    int knight = (attacking_side == white) ? N : n;
    if (knight_attacks[square] & bitboards[knight]) return 1;

    // 3. Attacked by kings
    int king = (attacking_side == white) ? K : k;
    if (king_attacks[square] & bitboards[king]) return 1;

    // 4. Attacked by bishops or queens
    int bishop = (attacking_side == white) ? B : b;
    int queen = (attacking_side == white) ? Q : q;
    if (get_bishop_attacks(square, total_occupancy) & (bitboards[bishop] | bitboards[queen])) return 1;

    // 5. Attacked by rooks or queens
    int rook = (attacking_side == white) ? R : r;
    if (get_rook_attacks(square, total_occupancy) & (bitboards[rook] | bitboards[queen])) return 1;

    return 0; // The square is safe
}

int make_move(int move) {
    // Create local copies of the current board state in case the move is illegal
    U64 bitboards_copy[12];
    for (int i = 0; i < 12; i++) bitboards_copy[i] = bitboards[i];
    int side_copy = side;
    int enpassant_copy = enpassant;
    int castle_copy = castle;

    // Decode the move
    int source = GET_SOURCE(move);
    int target = GET_TARGET(move);
    int piece = GET_PIECE(move);
    int promoted = GET_PROMOTED(move);
    int capture = GET_CAPTURE(move);
    int double_push = GET_DOUBLE_PUSH(move);
    int enpass = GET_ENPASSANT(move);
    int castling = GET_CASTLING(move);

    // Move the piece
    pop_bit(bitboards[piece], source);
    set_bit(bitboards[piece], target);

    // Handle standard captures
    if (capture && !enpass) {
        int start_piece = (side == white) ? p : P;
        int end_piece = (side == white) ? k : K;
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
            if (get_bit(bitboards[bb_piece], target)) {
                pop_bit(bitboards[bb_piece], target);
                break;
            }
        }
    }

    // Handle pawn promotions
    if (promoted) {
        pop_bit(bitboards[piece], target);
        set_bit(bitboards[promoted], target);
    }

    // Handle en passant captures
    if (enpass) {
        if (side == white) pop_bit(bitboards[p], target + 8);
        else pop_bit(bitboards[P], target - 8);
    }

    enpassant = -1; // Reset en passant square 

    // Handle double pawn pushes
    if (double_push) {
        if (side == white) enpassant = target + 8;
        else enpassant = target - 8;
    }

    // Handle castling (moving the rook to jump over the king)
    if (castling) {
        switch (target) {
            case g1: // White kingside
                pop_bit(bitboards[R], h1);
                set_bit(bitboards[R], f1);
                break;
            case c1: // White queenside
                pop_bit(bitboards[R], a1);
                set_bit(bitboards[R], d1);
                break;
            case g8: // Black kingside
                pop_bit(bitboards[r], h8);
                set_bit(bitboards[r], f8);
                break;
            case c8: // Black queenside
                pop_bit(bitboards[r], a8);
                set_bit(bitboards[r], d8);
                break;
        }
    }

    // Update castling rights based on piece movement
    castle &= castling_rights[source];
    castle &= castling_rights[target];

    // Change turns
    side ^= 1;

    // Legality Check: Ensure the king is not left in check
    int king_square = (side == black) ? get_lsb_index(bitboards[K]) : get_lsb_index(bitboards[k]);
    
    if (is_square_attacked(king_square, side)) {
        // Illegal move! The king is in check. Restore the board.
        for (int i = 0; i < 12; i++) bitboards[i] = bitboards_copy[i];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;
        return 0;
    }

    return 1; // Legal move!
}

void init_leapers() {
    for (int square = 0; square < 64; square++) {
        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
        
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    }
}

void generate_moves(MoveList &move_list, int side) {
    // Generate occupancies
    U64 white_occupancy = bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K];
    U64 black_occupancy = bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k];
    U64 total_occupancy = white_occupancy | black_occupancy;
    U64 friendly_occupancy = (side == white) ? white_occupancy : black_occupancy;
    U64 enemy_occupancy = (side == white) ? black_occupancy : white_occupancy;

    // Generate Pawn Moves
    int pawn_piece = (side == white) ? P : p;
    U64 pawns = bitboards[pawn_piece];
    while (pawns) {
        int source = get_lsb_index(pawns);
        
        // Forward pushes
        int target = (side == white) ? source - 8 : source + 8;
        if (!(total_occupancy & (1ULL << target))) { // If square ahead is empty
            // Promotions
            if ((side == white && source >= a7 && source <= h7) || (side == black && source >= a2 && source <= h2)) {
                move_list.add_move(ENCODE_MOVE(source, target, pawn_piece, (side == white) ? Q : q, 0, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source, target, pawn_piece, (side == white) ? R : r, 0, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source, target, pawn_piece, (side == white) ? B : b, 0, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source, target, pawn_piece, (side == white) ? N : n, 0, 0, 0, 0));
            } else {
                move_list.add_move(ENCODE_MOVE(source, target, pawn_piece, 0, 0, 0, 0, 0));
                
                // Double Pushes
                if ((side == white && source >= a2 && source <= h2) || (side == black && source >= a7 && source <= h7)) {
                    int double_target = (side == white) ? source - 16 : source + 16;
                    if (!(total_occupancy & (1ULL << double_target))) {
                        move_list.add_move(ENCODE_MOVE(source, double_target, pawn_piece, 0, 0, 1, 0, 0));
                    }
                }
            }
        }
        
        // Pawn Captures
        U64 attacks = pawn_attacks[side][source] & enemy_occupancy;
        while (attacks) {
            int attack_target = get_lsb_index(attacks);
            if ((side == white && source >= a7 && source <= h7) || (side == black && source >= a2 && source <= h2)) {
                move_list.add_move(ENCODE_MOVE(source, attack_target, pawn_piece, (side == white) ? Q : q, 1, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source, attack_target, pawn_piece, (side == white) ? R : r, 1, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source, attack_target, pawn_piece, (side == white) ? B : b, 1, 0, 0, 0));
                move_list.add_move(ENCODE_MOVE(source, attack_target, pawn_piece, (side == white) ? N : n, 1, 0, 0, 0));
            } else {
                move_list.add_move(ENCODE_MOVE(source, attack_target, pawn_piece, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, attack_target);
        }

        // En Passant Captures
        if (enpassant != -1) {
            U64 ep_attacks = pawn_attacks[side][source] & (1ULL << enpassant);
            if (ep_attacks) {
                move_list.add_move(ENCODE_MOVE(source, enpassant, pawn_piece, 0, 1, 0, 1, 0));
            }
        }
        
        pop_bit(pawns, source);
    }

    // Generate Moves for Knights, Bishops, Rooks, Queens, and Kings
    int start_piece = (side == white) ? N : n;
    int end_piece = (side == white) ? K : k;

    for (int piece = start_piece; piece <= end_piece; piece++) {
        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int source = get_lsb_index(bitboard);
            U64 attacks = 0ULL;
            
            // Get attack mask based on piece type
            if (piece == N || piece == n) attacks = knight_attacks[source];
            else if (piece == B || piece == b) attacks = get_bishop_attacks(source, total_occupancy);
            else if (piece == R || piece == r) attacks = get_rook_attacks(source, total_occupancy);
            else if (piece == Q || piece == q) attacks = get_bishop_attacks(source, total_occupancy) | get_rook_attacks(source, total_occupancy);
            else if (piece == K || piece == k) attacks = king_attacks[source];
            
            // Remove squares occupied by our own team
            attacks &= ~friendly_occupancy;
            
            while (attacks) {
                int target = get_lsb_index(attacks);
                int capture = get_bit(enemy_occupancy, target);
                move_list.add_move(ENCODE_MOVE(source, target, piece, 0, capture, 0, 0, 0));
                pop_bit(attacks, target);
            }
            
            pop_bit(bitboard, source);
        }
    }
}