#include "tt.h"

TTEntry* tt_table = nullptr;

void init_tt() {
    // Dynamically allocate the memory heap
    tt_table = new TTEntry[TT_SIZE];
    clear_tt();
}

void clear_tt() {
    for (int i = 0; i < TT_SIZE; i++) {
        tt_table[i].hash_key = 0;
        tt_table[i].depth = 0;
        tt_table[i].flag = 0;
        tt_table[i].score = 0;
        tt_table[i].best_move = 0;
    }
}

int read_tt(int alpha, int beta, int depth, int &best_move) {
    // Find the correct index using modulo
    TTEntry *entry = &tt_table[hash_key % TT_SIZE];
    
    // Ensure this is the exact same position (no collisions)
    if (entry->hash_key == hash_key) {
        // Always extract the best move for Move Ordering, even if depth is low!
        best_move = entry->best_move;
        
        // If we have searched this position deeply enough, we can use the score to skip!
        if (entry->depth >= depth) {
            if (entry->flag == HASH_EXACT) return entry->score;
            if (entry->flag == HASH_ALPHA && entry->score <= alpha) return alpha;
            if (entry->flag == HASH_BETA && entry->score >= beta) return beta;
        }
    }
    return NO_HASH_ENTRY;
}

void write_tt(int depth, int score, int flag, int best_move) {
    TTEntry *entry = &tt_table[hash_key % TT_SIZE];
    
    // Always overwrite (for now) to prioritize the most recent positions
    entry->hash_key = hash_key;
    entry->depth = depth;
    entry->flag = flag;
    entry->score = score;
    entry->best_move = best_move;
}