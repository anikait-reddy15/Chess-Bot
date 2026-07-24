#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"

// Constants to prevent pieces from wrapping around the board during bitwise shifts
const U64 not_a_file = 0xFEFEFEFEFEFEFEFEULL; // All squares except File A
const U64 not_h_file = 0x7F7F7F7F7F7F7F7FULL; // All squares except File H
const U64 not_ab_file = 0xFCFCFCFCFCFCFCFCULL; // All squares except Files A and B
const U64 not_hg_file = 0x3F3F3F3F3F3F3F3FULL; // All squares except Files H and G

// Lookup tables for non-sliding piece attacks
extern U64 knight_attacks[64];
extern U64 king_attacks[64];

// Function to generate the attack masks for a single square
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);

// Function to pre-calculate all leaper attacks into the lookup tables
void init_leapers();

#endif