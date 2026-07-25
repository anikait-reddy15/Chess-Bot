#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "evaluate.h"
#include "search.h"
#include "uci.h"
#include "zobrist.h" // Include Zobrist initialization

int main() {
    // 1. Initialize Zobrist random keys
    init_zobrist();

    // 2. Initialize lookup tables for move generation
    init_leapers(); 
    
    // 3. Hand control over to the UCI loop
    uci_loop();
    
    return 0;
}