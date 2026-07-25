#include "search.h"
#include "bitboard.h"
#include "movegen.h"
#include "evaluate.h"
#include <iostream>

// Track search statistics
int search_nodes = 0;
int best_move_found = 0;

// Scores a move for MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
int score_move(int move) {
    int score = 0;
    
    // 1. Promotions get a massive bonus
    int promoted = GET_PROMOTED(move);
    if (promoted) {
        score += 10000 + piece_weights[promoted]; 
    }
    
    // 2. Captures get scored based on the pieces involved
    if (GET_CAPTURE(move)) {
        int target = GET_TARGET(move);
        int attacker = GET_PIECE(move);
        int victim = P; 
        
        int start_piece = (side == white) ? p : P;
        int end_piece = (side == white) ? k : K;
        
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
            if (get_bit(bitboards[bb_piece], target)) {
                victim = bb_piece;
                break;
            }
        }
        
        score += 10000 + piece_weights[victim] - piece_weights[attacker];
    }
    
    return score;
}

// Sorts the move list
void sort_moves(MoveList &move_list) {
    int scores[256];
    for (int i = 0; i < move_list.count; i++) {
        scores[i] = score_move(move_list.moves[i]);
    }
    
    for (int i = 0; i < move_list.count; i++) {
        int max_idx = i;
        for (int j = i + 1; j < move_list.count; j++) {
            if (scores[j] > scores[max_idx]) {
                max_idx = j;
            }
        }
        
        int temp_score = scores[i];
        scores[i] = scores[max_idx];
        scores[max_idx] = temp_score;
        
        int temp_move = move_list.moves[i];
        move_list.moves[i] = move_list.moves[max_idx];
        move_list.moves[max_idx] = temp_move;
    }
}

// Quiescence Search
int quiescence(int alpha, int beta) {
    search_nodes++;
    
    int evaluation = evaluate_position();
    
    if (evaluation >= beta) {
        return beta;
    }
    if (evaluation > alpha) {
        alpha = evaluation;
    }
    
    MoveList move_list;
    generate_moves(move_list, side);
    sort_moves(move_list);
    
    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];
        
        if (!GET_CAPTURE(move)) continue;
        
        U64 bitboards_copy[12];
        for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;
        
        if (make_move(move) == 0) continue;
        
        int score = -quiescence(-beta, -alpha);
        
        for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;
        
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    
    return alpha;
}

// Core Negamax Search
int negamax(int alpha, int beta, int depth) {
    if (depth == 0) {
        return quiescence(alpha, beta);
    }

    search_nodes++;

    MoveList move_list;
    generate_moves(move_list, side);
    sort_moves(move_list);

    int legal_moves = 0;

    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];

        U64 bitboards_copy[12];
        for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;

        if (make_move(move) == 0) continue;

        legal_moves++;
        int score = -negamax(-beta, -alpha, depth - 1);

        for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    if (legal_moves == 0) {
        int king_square = (side == white) ? get_lsb_index(bitboards[K]) : get_lsb_index(bitboards[k]);
        if (is_square_attacked(king_square, side ^ 1)) {
            return -49000 + (100 - depth);
        } else {
            return 0;
        }
    }

    return alpha;
}

// Main Search Trigger with Iterative Deepening
void search_position(int max_depth) {
    int best_move_overall = 0;
    
    // Iterative Deepening Loop
    for (int current_depth = 1; current_depth <= max_depth; current_depth++) {
        search_nodes = 0;
        best_move_found = 0;
        
        MoveList move_list;
        generate_moves(move_list, side);
        sort_moves(move_list);

        int alpha = -50000;
        int beta = 50000;

        for (int i = 0; i < move_list.count; i++) {
            int move = move_list.moves[i];

            U64 bitboards_copy[12];
            for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
            int side_copy = side;
            int enpassant_copy = enpassant;
            int castle_copy = castle;

            if (make_move(move) == 0) continue; 

            int score = -negamax(-beta, -alpha, current_depth - 1);

            for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
            side = side_copy;
            enpassant = enpassant_copy;
            castle = castle_copy;

            if (score > alpha) {
                alpha = score;
                best_move_found = move;
            }
        }
        
        // Save the best move from this depth to play if we need to abort
        if (best_move_found != 0) {
            best_move_overall = best_move_found;
        }

        // Print UCI info stream for the current depth
        if (best_move_overall) {
            int source = GET_SOURCE(best_move_overall);
            int target = GET_TARGET(best_move_overall);
            int promoted = GET_PROMOTED(best_move_overall);
            
            std::string move_string = "";
            move_string += (char)('a' + (source % 8));
            move_string += (char)('8' - (source / 8));
            move_string += (char)('a' + (target % 8));
            move_string += (char)('8' - (target / 8));
            
            if (promoted) {
                if (promoted == Q || promoted == q) move_string += 'q';
                if (promoted == R || promoted == r) move_string += 'r';
                if (promoted == B || promoted == b) move_string += 'b';
                if (promoted == N || promoted == n) move_string += 'n';
            }

            std::cout << "info depth " << current_depth << " score cp " << alpha << " nodes " << search_nodes << "\n";
        }
    }

    // Print final UCI bestmove once the target depth is fully completed
    if (best_move_overall) {
        int source = GET_SOURCE(best_move_overall);
        int target = GET_TARGET(best_move_overall);
        int promoted = GET_PROMOTED(best_move_overall);
        
        std::string move_string = "";
        move_string += (char)('a' + (source % 8));
        move_string += (char)('8' - (source / 8));
        move_string += (char)('a' + (target % 8));
        move_string += (char)('8' - (target / 8));
        
        if (promoted) {
            if (promoted == Q || promoted == q) move_string += 'q';
            if (promoted == R || promoted == r) move_string += 'r';
            if (promoted == B || promoted == b) move_string += 'b';
            if (promoted == N || promoted == n) move_string += 'n';
        }

        std::cout << "bestmove " << move_string << "\n";
    }
}