#include "nnue.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

NeuralNetwork nn;
bool nnue_loaded = false;

// Loads a trained .bin file containing the weights into our arrays
bool load_nnue_weights(std::string filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "info string Warning: NNUE weights not found at " << filepath << ". Falling back to PST.\n";
        return false;
    }

    // Read the binary data straight into our structs (requires a specifically formatted .bin file)
    file.read(reinterpret_cast<char*>(&nn.weights_input_hidden), sizeof(nn.weights_input_hidden));
    file.read(reinterpret_cast<char*>(&nn.biases_hidden), sizeof(nn.biases_hidden));
    file.read(reinterpret_cast<char*>(&nn.weights_hidden_output), sizeof(nn.weights_hidden_output));
    file.read(reinterpret_cast<char*>(&nn.bias_output), sizeof(nn.bias_output));

    file.close();
    nnue_loaded = true;
    std::cout << "info string NNUE weights successfully loaded!\n";
    return true;
}

// The Forward Pass (Inference)
int evaluate_nnue() {
    float hidden_layer[HIDDEN_SIZE];
    
    // 1. Initialize hidden layer with biases
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        hidden_layer[i] = nn.biases_hidden[i];
    }

    // 2. Propagate Inputs to Hidden Layer
    // Instead of looping 768 times, we ONLY loop through the pieces actually on the board!
    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = bitboards[piece];
        while (bitboard) {
            int square = get_lsb_index(bitboard);
            
            int mapped_piece = piece;
            int mapped_square = square;

            // PERSPECTIVE FLIP: If it is Black's turn, flip the board vertically 
            // and swap the piece colors so the AI always thinks it is playing White!
            if (side == black) {
                mapped_square ^= 56; 
                mapped_piece = (piece < 6) ? piece + 6 : piece - 6; 
            }

            // Calculate the unique input index (0 to 767) using the mapped data
            int input_index = (mapped_piece * 64) + mapped_square;

            // Add this piece's weights to the hidden layer
            for (int i = 0; i < HIDDEN_SIZE; i++) {
                hidden_layer[i] += nn.weights_input_hidden[input_index][i];
            }

            pop_bit(bitboard, square);
        }
    }

    // 3. Apply ReLU Activation and propagate to Output Layer
    float output = nn.bias_output;
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        // ReLU (Rectified Linear Unit): If negative, set to 0.
        float relu_activation = std::max(0.0f, hidden_layer[i]);
        
        output += relu_activation * nn.weights_hidden_output[i];
    }

    // 4. Return the score from the perspective of the side to move
    int centipawn_score = static_cast<int>(output);
    return (side == white) ? centipawn_score : -centipawn_score;
}