#ifndef MOVELIST_H
#define MOVELIST_H

#include "move.h"

// A fast, stack-allocated structure to hold generated moves
struct MoveList {
    // Array to hold up to 256 encoded moves
    int moves[256];
    
    // Counter to track how many moves have been generated
    int count = 0;

    // Helper function to push a move into the list and increment the counter
    inline void add_move(int move) {
        moves[count] = move;
        count++;
    }
};

#endif