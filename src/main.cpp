#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "evaluate.h"
#include "search.h"
#include "uci.h"
#include "zobrist.h"
#include "tt.h" 

int main() {
    init_zobrist();
    init_tt();      
    init_leapers(); 
    uci_loop();
    return 0;
}