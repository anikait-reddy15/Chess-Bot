#include "evaluate.h"
#include "bitboard.h"
#include "nnue.h"

// 100 centipawns = 1 pawn
const int piece_weights[] = {
    100, 320, 330, 500, 900, 20000, // White: P, N, B, R, Q, K
    100, 320, 330, 500, 900, 20000  // Black: p, n, b, r, q, k
};

// Pawns are encouraged to control the center and push towards promotion
const int pawn_pst[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     50,  50,  50,  50,  50,  50,  50,  50,
     10,  10,  20,  30,  30,  20,  10,  10,
      5,   5,  10,  25,  25,  10,   5,   5,
      0,   0,   0,  20,  20,   0,   0,   0,
      5,  -5, -10,   0,   0, -10,  -5,   5,
      5,  10,  10, -20, -20,  10,  10,   5,
      0,   0,   0,   0,   0,   0,   0,   0
};

// Knights are terrible on the edges, and brilliant in the center
const int knight_pst[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

// Bishops want to control long diagonals
const int bishop_pst[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,  5,  10,  10,  5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

// Rooks are encouraged to take the 7th rank and central files
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

// Queens are slightly encouraged to stay centralized but mostly rely on tactics
const int queen_pst[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10,   0,   0,  0,  0,   0,   0, -10,
    -10,   0,   5,  5,  5,   5,   0, -10,
     -5,   0,   5,  5,  5,   5,   0,  -5,
      0,   0,   5,  5,  5,   5,   0,  -5,
    -10,   5,   5,  5,  5,   5,   0, -10,
    -10,   0,   5,  0,  0,   0,   0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

// Kings are heavily penalized for staying in the center, encouraging castling
const int king_pst[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20
};

// Array of pointers to easily access the tables based on piece type (0-5)
const int* pst_pointers[] = { 
    pawn_pst, knight_pst, bishop_pst, rook_pst, queen_pst, king_pst 
};

int evaluate_position() {
    // 1. If Neural Network is active, bypass this entire function!
    if (nnue_loaded) {
        return evaluate_nnue();
    }

    // --- Fallback to Hand-Crafted Evaluation ---
    int score = 0;

    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = bitboards[piece];
        
        while (bitboard) {
            int square = get_lsb_index(bitboard);
            int material_score = piece_weights[piece];
            
            int piece_type = piece % 6; 
            int positional_score = 0;

            if (piece >= P && piece <= K) {
                // White gets the score directly
                positional_score = pst_pointers[piece_type][square];
                score += (material_score + positional_score);
            } else {
                // Black must mirror the square so they get the same positional logic
                int mirrored_square = square ^ 56;
                positional_score = pst_pointers[piece_type][mirrored_square];
                score -= (material_score + positional_score);
            }
            pop_bit(bitboard, square);
        }
    }

    // Always return the score relative to the side to move (for Negamax)
    return (side == white) ? score : -score;
}