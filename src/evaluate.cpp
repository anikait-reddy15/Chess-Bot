#include "evaluate.h"
#include "bitboard.h"

// 100 centipawns = 1 pawn
const int piece_weights[] = {
    100, 320, 330, 500, 900, 20000, // White: P, N, B, R, Q, K
    100, 320, 330, 500, 900, 20000  // Black: p, n, b, r, q, k
};

// Piece-Square Tables (PST) for positional evaluation
const int pawn_pst[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
     50, 50, 50, 50, 50, 50, 50, 50,
     10, 10, 20, 30, 30, 20, 10, 10,
      5,  5, 10, 25, 25, 10,  5,  5,
      0,  0,  0, 20, 20,  0,  0,  0,
      5, -5,-10,  0,  0,-10, -5,  5,
      5, 10, 10,-20,-20, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
};

const int knight_pst[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishop_pst[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int rook_pst[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0
};

// Array of pointers to easily access the tables based on piece type
const int* pst_pointers[] = { 
    pawn_pst, knight_pst, bishop_pst, rook_pst, bishop_pst, pawn_pst // Q and K tables simplified for now
};

int evaluate_position() {
    int score = 0;

    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = bitboards[piece];
        
        while (bitboard) {
            // Extract the square of the current piece
            int square = get_lsb_index(bitboard);
            
            // Get material weight
            int material_score = piece_weights[piece];
            
            // Get positional score
            int piece_type = piece % 6; // Normalizes piece to 0-5 (P to K)
            int positional_score = 0;

            if (piece >= P && piece <= K) {
                // White piece: read directly from table
                positional_score = pst_pointers[piece_type][square];
                score += (material_score + positional_score);
            } else {
                // Black piece: mirror the square vertically (e.g., a8 becomes a1)
                int mirrored_square = square ^ 56;
                positional_score = pst_pointers[piece_type][mirrored_square];
                score -= (material_score + positional_score);
            }
            
            // Pop the bit to move to the next piece on this bitboard
            pop_bit(bitboard, square);
        }
    }

    return (side == white) ? score : -score;
}