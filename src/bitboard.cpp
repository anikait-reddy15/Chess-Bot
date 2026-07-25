#include "bitboard.h"

// Initialize the 12 bitboards with the standard chess starting position
U64 bitboards[12] = {
    0x00FF000000000000ULL, // P (White Pawns)
    0x4200000000000000ULL, // N (White Knights)
    0x2400000000000000ULL, // B (White Bishops)
    0x8100000000000000ULL, // R (White Rooks)
    0x0800000000000000ULL, // Q (White Queen)
    0x1000000000000000ULL, // K (White King)
    0x000000000000FF00ULL, // p (Black Pawns)
    0x0000000000000042ULL, // n (Black Knights)
    0x0000000000000024ULL, // b (Black Bishops)
    0x0000000000000081ULL, // r (Black Rooks)
    0x0000000000000008ULL, // q (Black Queen)
    0x0000000000000010ULL  // k (Black King)
};

// Initialize board state variables
int side = white;
int enpassant = -1; // -1 means no en passant square available
int castle = 15;    // Binary 1111 represents all 4 castling rights are intact

void print_bitboard(U64 bitboard) {
    std::cout << "\n";
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (!file) std::cout << 8 - rank << "  ";
            std::cout << get_bit(bitboard, square) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
    std::cout << "   Bitboard: " << bitboard << "\n\n";
}

void print_board() {
    std::cout << "\n";
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            
            // Print rank numbers on the left
            if (!file) {
                std::cout << 8 - rank << "  ";
            }
            
            // Variable to track which piece occupies the square
            int piece = -1;
            
            // Loop through all 12 bitboards to see if a piece is on this square
            for (int bb_piece = P; bb_piece <= k; bb_piece++) {
                if (get_bit(bitboards[bb_piece], square)) {
                    piece = bb_piece;
                    break; // Found the piece, no need to check other bitboards
                }
            }
            
            // If piece is -1, the square is empty, print a dot
            // Otherwise, print the corresponding ASCII character
            if (piece == -1) {
                std::cout << ". ";
            } else {
                std::cout << ascii_pieces[piece] << " ";
            }
        }
        std::cout << "\n";
    }
    // Print file letters at the bottom
    std::cout << "\n   a b c d e f g h\n\n";
    
    // Print current state info below the board
    std::cout << "   Side to move: " << (side == white ? "White" : "Black") << "\n";
    std::cout << "   En Passant: " << (enpassant != -1 ? std::to_string(enpassant) : "None") << "\n";
    std::cout << "   Castling Rights: " << castle << "\n\n";
}

void parse_fen(std::string fen) {
    // Reset the board state
    for (int i = 0; i < 12; i++) bitboards[i] = 0ULL;
    side = white;
    enpassant = -1;
    castle = 0;
    
    int square = 0;
    int i = 0;
    
    // 1. Parse piece placement
    while (i < fen.length() && fen[i] != ' ') {
        if (fen[i] == '/') {
            i++;
            continue;
        }
        // If it's a number, skip that many empty squares
        if (fen[i] >= '1' && fen[i] <= '8') {
            square += (fen[i] - '0');
        } else {
            // It's a piece character, match it and set the bit
            int piece = -1;
            switch (fen[i]) {
                case 'P': piece = P; break;
                case 'N': piece = N; break;
                case 'B': piece = B; break;
                case 'R': piece = R; break;
                case 'Q': piece = Q; break;
                case 'K': piece = K; break;
                case 'p': piece = p; break;
                case 'n': piece = n; break;
                case 'b': piece = b; break;
                case 'r': piece = r; break;
                case 'q': piece = q; break;
                case 'k': piece = k; break;
            }
            if (piece != -1) {
                set_bit(bitboards[piece], square);
                square++;
            }
        }
        i++;
    }
    
    i++; // Skip the space
    
    // 2. Parse side to move
    if (i < fen.length()) {
        side = (fen[i] == 'w') ? white : black;
        i += 2; // Skip 'w' or 'b' and the following space
    }
    
    // 3. Parse castling rights
    while (i < fen.length() && fen[i] != ' ') {
        switch (fen[i]) {
            case 'K': castle |= 1; break; // Bit 1
            case 'Q': castle |= 2; break; // Bit 2
            case 'k': castle |= 4; break; // Bit 3
            case 'q': castle |= 8; break; // Bit 4
            case '-': break; // No castling rights
        }
        i++;
    }
    
    i++; // Skip the space
    
    // 4. Parse en passant square
    if (i < fen.length() && fen[i] != '-') {
        int file = fen[i] - 'a';
        int rank = 8 - (fen[i+1] - '0');
        enpassant = rank * 8 + file;
    } else {
        enpassant = -1;
    }
}