#include "bitboard.h"
#include "zobrist.h"

U64 bitboards[12] = {
    0x00FF000000000000ULL, 0x4200000000000000ULL, 0x2400000000000000ULL, 
    0x8100000000000000ULL, 0x0800000000000000ULL, 0x1000000000000000ULL, 
    0x000000000000FF00ULL, 0x0000000000000042ULL, 0x0000000000000024ULL, 
    0x0000000000000081ULL, 0x0000000000000008ULL, 0x0000000000000010ULL  
};

int side = white;
int enpassant = -1; 
int castle = 15;    
U64 hash_key = 0ULL; // Initialize the hash key

// Generate a unique 64-bit key by XORing the state of the board
U64 generate_hash_key() {
    U64 final_key = 0ULL;
    
    // 1. XOR the pieces
    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int square = get_lsb_index(bitboard);
            final_key ^= piece_keys[piece][square];
            pop_bit(bitboard, square);
        }
    }
    
    // 2. XOR the en passant square
    if (enpassant != -1) {
        final_key ^= enpassant_keys[enpassant];
    }
    
    // 3. XOR the castling rights
    final_key ^= castle_keys[castle];
    
    // 4. XOR the side to move (only if black)
    if (side == black) {
        final_key ^= side_key;
    }
    
    return final_key;
}

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
            if (!file) std::cout << 8 - rank << "  ";
            
            int piece = -1;
            for (int bb_piece = P; bb_piece <= k; bb_piece++) {
                if (get_bit(bitboards[bb_piece], square)) {
                    piece = bb_piece;
                    break; 
                }
            }
            if (piece == -1) std::cout << ". ";
            else std::cout << ascii_pieces[piece] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
    std::cout << "   Side to move: " << (side == white ? "White" : "Black") << "\n";
    std::cout << "   En Passant: " << (enpassant != -1 ? std::to_string(enpassant) : "None") << "\n";
    std::cout << "   Castling Rights: " << castle << "\n";
    std::cout << "   Hash Key: " << std::hex << hash_key << std::dec << "\n\n";
}

void parse_fen(std::string fen) {
    for (int i = 0; i < 12; i++) bitboards[i] = 0ULL;
    side = white;
    enpassant = -1;
    castle = 0;
    
    int square = 0;
    int i = 0;
    
    while (i < fen.length() && fen[i] != ' ') {
        if (fen[i] == '/') { i++; continue; }
        if (fen[i] >= '1' && fen[i] <= '8') {
            square += (fen[i] - '0');
        } else {
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
    i++; 
    
    if (i < fen.length()) {
        side = (fen[i] == 'w') ? white : black;
        i += 2; 
    }
    
    while (i < fen.length() && fen[i] != ' ') {
        switch (fen[i]) {
            case 'K': castle |= 1; break; 
            case 'Q': castle |= 2; break; 
            case 'k': castle |= 4; break; 
            case 'q': castle |= 8; break; 
            case '-': break; 
        }
        i++;
    }
    i++; 
    
    if (i < fen.length() && fen[i] != '-') {
        int file = fen[i] - 'a';
        int rank = 8 - (fen[i+1] - '0');
        enpassant = rank * 8 + file;
    } else {
        enpassant = -1;
    }

    // After parsing the FEN, immediately generate the hash key
    hash_key = generate_hash_key();
}