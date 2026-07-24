#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"

// Constants to prevent pieces from wrapping around the board
const U64 not_a_file = 0xFEFEFEFEFEFEFEFEULL;
const U64 not_h_file = 0x7F7F7F7F7F7F7F7FULL;
const U64 not_ab_file = 0xFCFCFCFCFCFCFCFCULL;
const U64 not_hg_file = 0x3F3F3F3F3F3F3F3FULL;

// Side to move encoding
enum { white, black };

// Lookup tables for leaper attacks
extern U64 knight_attacks[64];
extern U64 king_attacks[64];

// Lookup tables for pawn attacks [color][square]
extern U64 pawn_attacks[2][64];

// Functions to generate attack masks for a single square
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_pawn_attacks(int side, int square);

// Function to pre-calculate all leaper and pawn attacks
void init_leapers();

#endif