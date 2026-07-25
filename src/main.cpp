#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "move.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    init_leapers();
    
    std::cout << "\n--- Make Move Test ---\n";
    
    std::cout << "Board before move:\n";
    print_board();
    
    // Encode a double pawn push: White pawn from e2 to e4
    // ENCODE_MOVE(source, target, piece, promoted, capture, double_push, enpassant, castling)
    int test_move = ENCODE_MOVE(e2, e4, P, 0, 0, 1, 0, 0);
    
    std::cout << "Attempting to make move (e2 to e4)...\n";
    
    if (make_move(test_move)) {
        std::cout << "Move was LEGAL. Board updated.\n";
        print_board();
    } else {
        std::cout << "Move was ILLEGAL. Board reverted.\n";
    }
    
    return 0;
}