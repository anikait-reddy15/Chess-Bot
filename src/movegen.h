#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"
#include "movelist.h" // Include the move list to pass into the generator

// Constants to prevent pieces from wrapping around the board
const U64 not_a_file = 0xFEFEFEFEFEFEFEFEULL;
const U64 not_h_file = 0x7F7F7F7F7F7F7F7FULL;
const U64 not_ab_file = 0xFCFCFCFCFCFCFCFCULL;
const U64 not_hg_file = 0x3F3F3F3F3F3F3F3FULL;

// Lookup tables for non-sliding attacks
extern U64 knight_attacks[64];
extern U64 king_attacks[64];

// Lookup tables for pawn attacks [color][square]
extern U64 pawn_attacks[2][64];

// Castling rights update table
extern const int castling_rights[64];

// Functions to generate attack masks for a single square (Leapers & Pawns)

// Functions to generate attack masks for sliding pieces dynamically
U64 get_bishop_attacks(int square, U64 block);
U64 get_rook_attacks(int square, U64 block);

// Function to check if a specific square is attacked by a given side
int is_square_attacked(int square, int side);

// Core function to make a move on the board
int make_move(int move);

// Initialization function to pre-calculate all static attacks
void init_leapers();

// Core move generation function
void generate_moves(MoveList &move_list, int side);

#endif  