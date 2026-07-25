#ifndef MOVE_H
#define MOVE_H

#include <iostream>
#include "bitboard.h"

// Macro to compress all move details into a 32-bit integer
#define ENCODE_MOVE(source, target, piece, promoted, capture, double_push, enpassant, castling) \
    ((source) | \
    ((target) << 6) | \
    ((piece) << 12) | \
    ((promoted) << 16) | \
    ((capture) << 20) | \
    ((double_push) << 21) | \
    ((enpassant) << 22) | \
    ((castling) << 23))

// Macros to extract (decode) specific details from the 32-bit integer
// 0x3f is 63 in decimal (6 bits of 1s)
// 0xf is 15 in decimal (4 bits of 1s)
#define GET_SOURCE(move) ((move) & 0x3f)
#define GET_TARGET(move) (((move) >> 6) & 0x3f)
#define GET_PIECE(move) (((move) >> 12) & 0xf)
#define GET_PROMOTED(move) (((move) >> 16) & 0xf)
#define GET_CAPTURE(move) (((move) >> 20) & 1)
#define GET_DOUBLE_PUSH(move) (((move) >> 21) & 1)
#define GET_ENPASSANT(move) (((move) >> 22) & 1)
#define GET_CASTLING(move) (((move) >> 23) & 1)

// Function to print the move details in a human-readable format
inline void print_move(int move) {
    // Array to convert square indices back to string coordinates
    const char* square_coords[] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
    };

    std::cout << "\nMove Data: "
              << square_coords[GET_SOURCE(move)] << " to "
              << square_coords[GET_TARGET(move)]
              << " | Piece: " << ascii_pieces[GET_PIECE(move)]
              << " | Capture: " << GET_CAPTURE(move)
              << " | Castling: " << GET_CASTLING(move) << "\n";
}

#endif