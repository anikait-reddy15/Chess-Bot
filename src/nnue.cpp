#include "nnue.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

NeuralNetwork nn;
bool nnue_loaded = false;

bool load_nnue_weights(std::string filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "info string Warning: NNUE weights not found at " << filepath << ". Falling back to PST.\n";
        return false;
    }

    file.read(reinterpret_cast<char*>(&nn.weights_input_hidden), sizeof(nn.weights_input_hidden));
    file.read(reinterpret_cast<char*>(&nn.biases_hidden), sizeof(nn.biases_hidden));
    file.read(reinterpret_cast<char*>(&nn.weights_hidden_output), sizeof(nn.weights_hidden_output));
    file.read(reinterpret_cast<char*>(&nn.bias_output), sizeof(nn.bias_output));

    file.close();
    nnue_loaded = true;
    std::cout << "info string NNUE weights successfully loaded!\n";
    return true;
}

int evaluate_nnue() {
    float hidden_layer[HIDDEN_SIZE];
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        hidden_layer[i] = nn.biases_hidden[i];
    }

    // 1. Find the Friendly King Square
    int king_sq = (side == white) ? get_lsb_index(bitboards[K]) : get_lsb_index(bitboards[k]);
    if (side == black) king_sq ^= 56; // Flip the board for Black's perspective

    // 2. Setup the Piece Mapping (0-4 for Friendly, 5-9 for Enemy)
    int piece_map[12];
    for(int i=0; i<12; i++) piece_map[i] = -1; // Kings are -1 (ignored)

    if (side == white) {
        piece_map[P] = 0; piece_map[N] = 1; piece_map[B] = 2; piece_map[R] = 3; piece_map[Q] = 4;
        piece_map[p] = 5; piece_map[n] = 6; piece_map[b] = 7; piece_map[r] = 8; piece_map[q] = 9;
    } else {
        // From Black's perspective, Black pieces are Friendly (0-4), White are Enemy (5-9)
        piece_map[p] = 0; piece_map[n] = 1; piece_map[b] = 2; piece_map[r] = 3; piece_map[q] = 4;
        piece_map[P] = 5; piece_map[N] = 6; piece_map[B] = 7; piece_map[R] = 8; piece_map[Q] = 9;
    }

    // 3. Propagate Features to Hidden Layer
    for (int piece = P; piece <= k; piece++) {
        if (piece == K || piece == k) continue; // We don't map kings into the features, they are the anchor!

        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int square = get_lsb_index(bitboard);
            
            int mapped_square = (side == black) ? square ^ 56 : square;
            int mapped_piece = piece_map[piece];

            // The HalfKP Index Formula
            int input_index = (king_sq * 640) + (mapped_piece * 64) + mapped_square;

            for (int i = 0; i < HIDDEN_SIZE; i++) {
                hidden_layer[i] += nn.weights_input_hidden[input_index][i];
            }

            pop_bit(bitboard, square);
        }
    }

    // 4. Apply ReLU and Calculate Output
    float output = nn.bias_output;
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        float relu_activation = std::max(0.0f, hidden_layer[i]);
        output += relu_activation * nn.weights_hidden_output[i];
    }

    int centipawn_score = static_cast<int>(output);
    
    return centipawn_score;
}