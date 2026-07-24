#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <iostream>

// Define U64 as a standard 64-bit unsigned integer
typedef uint64_t U64;

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

// Function to visualize the bitboard in the terminal
void print_bitboard(U64 bitboard);

#endif