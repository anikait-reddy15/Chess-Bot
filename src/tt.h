#ifndef TT_H
#define TT_H

#include "bitboard.h"

// Flags to know what kind of score we stored during Alpha-Beta pruning
enum { HASH_EXACT, HASH_ALPHA, HASH_BETA };

// A massive constant to indicate no entry was found
const int NO_HASH_ENTRY = 100000;

// The structure of a single memory cell in our table
struct TTEntry {
    U64 hash_key;
    int depth;
    int flag;
    int score;
    int best_move;
};

// Allocate memory for ~4 million entries (roughly 64MB of RAM)
const int TT_SIZE = 0x400000;
extern TTEntry* tt_table;

void init_tt();
void clear_tt();
int read_tt(int alpha, int beta, int depth, int &best_move);
void write_tt(int depth, int score, int flag, int best_move);

#endif