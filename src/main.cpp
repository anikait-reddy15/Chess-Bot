#include <iostream>
#include "bitboard.h"
#include "movegen.h"
#include "move.h"
#include "movelist.h"

int main() {
    std::cout << "Project A Engine Initialized\n";
    init_leapers();
    
    std::cout << "\n--- Square Attack Detection Test ---\n";
    
    // Drop a Black Knight right next to the White King
    set_bit(bitboards[n], f3);
    
    std::cout << "Board after placing a Black Knight on f3:\n";
    print_board();
    
    // Check if the e1 square (where the White King starts) is attacked by Black
    if (is_square_attacked(e1, black)) {
        std::cout << "ALERT: The White King on e1 is in Check by Black!\n";
    } else {
        std::cout << "SAFE: e1 is not attacked.\n";
    }
    
    // Check if d4 is attacked by White (it should be, by the starting c2 and e2 pawns)
    if (is_square_attacked(d4, white)) {
        std::cout << "CONFIRMED: The d4 square is heavily guarded by White pawns.\n";
    }
    
    return 0;
}