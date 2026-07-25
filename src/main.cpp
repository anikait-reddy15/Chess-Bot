#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "move.h"
#include "movelist.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    init_leapers();
    
    // Create an empty MoveList
    MoveList move_list;
    
    std::cout << "\nGenerating Pseudo-Legal Moves for White Knights and Kings...\n";
    
    // Generate moves for the white side based on the starting position bitboards
    generate_moves(move_list, white);
    
    std::cout << "Total moves generated: " << move_list.count << "\n";
    
    // Print out all generated moves to verify them
    for (int i = 0; i < move_list.count; i++) {
        print_move(move_list.moves[i]);
    }
    
    return 0;
}