#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "bitboard.h"

// Zobrist keys for pieces [12 piece types][64 squares]
extern U64 piece_keys[12][64];

// Zobrist keys for en passant squares [64 squares]
extern U64 enpassant_keys[64];

// Zobrist keys for castling rights [16 possible combinations]
extern U64 castle_keys[16];

// Zobrist key for the side to move (applied only when it is Black's turn)
extern U64 side_key;

// Pseudo-Random Number Generator (PRNG) state
extern U64 random_state;

// Generates a random 64-bit integer
U64 get_random_U64();

// Initializes all Zobrist keys with random numbers
void init_zobrist();

#endif