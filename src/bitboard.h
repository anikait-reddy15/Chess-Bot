#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <iostream>

// Define U64 as a standard 64-bit unsigned integer
typedef uint64_t U64;

// Board square mapping (a8 = 0, h1 = 63)
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

// Piece encoding
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// ASCII characters for pieces (matches the enum order)
const char ascii_pieces[] = "PNBRQKpnbrqk";

// Global array to hold bitboards for all 12 piece types
extern U64 bitboards[12];

// Inline functions for fast bitwise operations
inline void set_bit(U64 &bitboard, int square) {
    bitboard |= (1ULL << square);
}

inline int get_bit(U64 bitboard, int square) {
    return (bitboard & (1ULL << square)) ? 1 : 0;
}

inline void pop_bit(U64 &bitboard, int square) {
    bitboard &= ~(1ULL << square);
}

// Brian Kernighan's Algorithm to count the number of 1s on a bitboard
inline int count_bits(U64 bitboard) {
    int count = 0;
    while (bitboard) {
        count++;
        bitboard &= bitboard - 1; // Resets the least significant 1-bit
    }
    return count;
}

// Extract the index of the Least Significant 1-Bit
inline int get_lsb_index(U64 bitboard) {
    if (bitboard) {
        // Fix for MSVC Warning C4146: using bitwise inversion instead of unary minus
        return count_bits((bitboard & ~(bitboard - 1)) - 1);
    }
    return -1;
}

// Function to visualize a specific bitboard
void print_bitboard(U64 bitboard);

// Function to visualize the entire board state
void print_board();

#endif