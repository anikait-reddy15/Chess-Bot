#ifndef NNUE_H
#define NNUE_H

#include "bitboard.h"
#include <string>

// HalfKP Architecture (King-Relative Features)
// 64 King Squares * 10 Piece Types (Friendly/Enemy P,N,B,R,Q) * 64 Piece Squares
const int INPUT_SIZE = 40960; 
const int HIDDEN_SIZE = 256;  

struct NeuralNetwork {
    float weights_input_hidden[INPUT_SIZE][HIDDEN_SIZE];
    float biases_hidden[HIDDEN_SIZE];
    float weights_hidden_output[HIDDEN_SIZE];
    float bias_output;
};

extern NeuralNetwork nn;
extern bool nnue_loaded;

bool load_nnue_weights(std::string filepath);
int evaluate_nnue();

#endif