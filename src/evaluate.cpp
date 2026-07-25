#include "evaluate.h"
#include "bitboard.h"

// 100 centipawns = 1 pawn
const int piece_weights[] = {
    100, 320, 330, 500, 900, 20000, // White: P, N, B, R, Q, K
    100, 320, 330, 500, 900, 20000  // Black: p, n, b, r, q, k
};

int evaluate_position() {
    int score = 0;

    // Loop through all 12 piece bitboards
    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = bitboards[piece];
        
        // Count how many pieces of this type exist
        int count = count_bits(bitboard);
        int material_score = count * piece_weights[piece];
        
        // White pieces add to the total score, Black pieces subtract
        if (piece >= P && piece <= K) {
            score += material_score;
        } else {
            score -= material_score;
        }
    }

    // Negamax perspective: Return the score relative to the side whose turn it is.
    // If it's White's turn, return the raw score.
    // If it's Black's turn, invert the score so Black knows positive is good for them.
    return (side == white) ? score : -score;
}