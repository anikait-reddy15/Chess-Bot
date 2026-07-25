#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "move.h" // Include our new encoding macros

int main() {
    std::cout << "Project A Engine Initialized\n";
    init_leapers();
    
    // Simulate a move: White Knight (N) moves from g4 to f6, capturing an enemy piece.
    // ENCODE_MOVE parameters: source, target, piece, promoted, capture, double_push, ep, castling
    int test_move = ENCODE_MOVE(g4, f6, N, 0, 1, 0, 0, 0);
    
    std::cout << "\nRaw 32-bit Integer Value of the Move: " << test_move;
    
    // Decode and print the human-readable version
    print_move(test_move);
    
    return 0;
}