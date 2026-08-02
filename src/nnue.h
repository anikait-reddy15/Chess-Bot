#ifndef NNUE_H
#define NNUE_H

#include "bitboard.h"
#include <string>

// Define the exact architecture of our Neural Network (Must match PyTorch)
const int INPUT_SIZE = 768;   // 64 squares * 12 piece types
const int HIDDEN_SIZE = 256;  // The hidden "thinking" layer

// Neural Network memory structures for inference
struct NeuralNetwork {
    // Layer 1: Inputs to Hidden Layer
    float weights_input_hidden[INPUT_SIZE][HIDDEN_SIZE];
    float biases_hidden[HIDDEN_SIZE];

    // Layer 2: Hidden Layer to Output Score
    float weights_hidden_output[HIDDEN_SIZE];
    float bias_output;
};

// Global instance of our network and a flag to check if it loaded successfully
extern NeuralNetwork nn;
extern bool nnue_loaded;

// Core NNUE functions
// Loads the .bin file exported from Python into the structs above
bool load_nnue_weights(std::string filepath);

// Performs the forward pass math to score the board
int evaluate_nnue();

#endif