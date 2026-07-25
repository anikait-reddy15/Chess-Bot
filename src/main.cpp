#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "move.h"
#include "movelist.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    init_leapers();
    
    // Simulate moving the e2 and d2 pawns by manually deleting them from the board
    // This will open up diagonals and files for the Bishops and Queen
    pop_bit(bitboards[P], e2);
    pop_bit(bitboards[P], d2);
    
    std::cout << "\nBoard after deleting e2 and d2 pawns:\n";
    print_board();
    
    // Create an empty MoveList
    MoveList move_list;
    
    std::cout << "\nGenerating Moves for White (Open Center)...\n";
    generate_moves(move_list, white);
    for (int i = 0; i < move_list.count; i++) {
        print_move(move_list.moves[i]);
    }
    
    return 0;
}