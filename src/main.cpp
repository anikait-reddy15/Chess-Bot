#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "evaluate.h"
#include "search.h"
#include "uci.h"
#include "zobrist.h"
#include "tt.h"
#include "nnue.h"

int main() {
    // 1. Initialize all lookup tables and random numbers
    init_zobrist();
    init_tt();      
    init_leapers(); 
    
    // 2. Load the Neural Network Brain!
    // If it fails, evaluate.cpp will automatically fallback to PSTs.
    load_nnue_weights("network.bin");

    // 3. Hand control over to the GUI/Terminal
    uci_loop();
    
    return 0;
}