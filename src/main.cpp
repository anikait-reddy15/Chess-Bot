#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "evaluate.h"
#include "search.h"
#include "uci.h" // Include our new UCI header

int main() {
    // Initialize lookup tables before doing anything else
    init_leapers(); 
    
    // Hand control over to the UCI loop
    uci_loop();
    
    return 0;
}