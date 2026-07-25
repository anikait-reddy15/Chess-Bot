#include "zobrist.h"

// Arrays to hold our random keys
U64 piece_keys[12][64];
U64 enpassant_keys[64];
U64 castle_keys[16];
U64 side_key;

// A simple but fast 64-bit Pseudo-Random Number Generator (Xorshift)
// Seeded with a specific number so our hashes are consistent across runs
U64 random_state = 1804289383ULL;

U64 get_random_U64() {
    U64 number = random_state;
    
    // Xorshift algorithm
    number ^= number << 13;
    number ^= number >> 7;
    number ^= number << 17;
    
    random_state = number;
    return number;
}

void init_zobrist() {
    // Fill piece keys
    for (int piece = P; piece <= k; piece++) {
        for (int square = 0; square < 64; square++) {
            piece_keys[piece][square] = get_random_U64();
        }
    }
    
    // Fill en passant keys
    for (int square = 0; square < 64; square++) {
        enpassant_keys[square] = get_random_U64();
    }
    
    // Fill castling keys
    for (int i = 0; i < 16; i++) {
        castle_keys[i] = get_random_U64();
    }
    
    // Fill side key
    side_key = get_random_U64();
}