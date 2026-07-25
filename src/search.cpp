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
    
    // 1. Promotions get a massive bonus (prioritize Q > R > B > N)
    int promoted = GET_PROMOTED(move);
    if (promoted) {
        score += 10000 + piece_weights[promoted]; 
    }
    
    // 2. Captures get scored based on the pieces involved
    if (GET_CAPTURE(move)) {
        int target = GET_TARGET(move);
        int attacker = GET_PIECE(move);
        int victim = P; // Default to pawn (handles En Passant correctly)
        
        // Find which enemy piece is sitting on the target square
        int start_piece = (side == white) ? p : P;
        int end_piece = (side == white) ? k : K;
        
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
            if (get_bit(bitboards[bb_piece], target)) {
                victim = bb_piece;
                break;
            }
        }
        
        // Score = Victim Value - Attacker Value + 10000 
        // (Ensures captures are searched before quiet moves)
        score += 10000 + piece_weights[victim] - piece_weights[attacker];
    }
    
    return score; // Quiet moves return 0
}

// Sorts the move list using a simple selection sort
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
        
        // Swap scores
        int temp_score = scores[i];
        scores[i] = scores[max_idx];
        scores[max_idx] = temp_score;
        
        // Swap moves
        int temp_move = move_list.moves[i];
        move_list.moves[i] = move_list.moves[max_idx];
        move_list.moves[max_idx] = temp_move;
    }
}

// Quiescence Search: Only looks at captures to prevent the Horizon Effect
int quiescence(int alpha, int beta) {
    search_nodes++;
    
    // Evaluate the current position
    int evaluation = evaluate_position();
    
    // Hard beta cutoff (Stand Pat)
    if (evaluation >= beta) {
        return beta;
    }
    
    // Update alpha if the current evaluation is better
    if (evaluation > alpha) {
        alpha = evaluation;
    }
    
    MoveList move_list;
    generate_moves(move_list, side);
    
    // Sort moves to search the best captures first!
    sort_moves(move_list);
    
    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];
        
        // ONLY look at captures in Quiescence search
        if (!GET_CAPTURE(move)) continue;
        
        // Create local copies of the board state to restore after recursive search
        U64 bitboards_copy[12];
        for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;
        
        // Make the move
        if (make_move(move) == 0) {
            continue; // Illegal move, board was restored automatically
        }
        
        // Recursively call quiescence with flipped scores
        int score = -quiescence(-beta, -alpha);
        
        // Restore board state
        for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;
        
        // Alpha-Beta Pruning
        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

// Core Negamax Search with Alpha-Beta Pruning
int negamax(int alpha, int beta, int depth) {
    // If we hit the depth limit, drop into Quiescence Search instead of returning static evaluation
    if (depth == 0) {
        return quiescence(alpha, beta);
    }

    search_nodes++;

    MoveList move_list;
    generate_moves(move_list, side);
    
    // Sort moves to find cutoffs instantly!
    sort_moves(move_list);

    int legal_moves = 0;

    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];

        // Create local copies of the board state
        U64 bitboards_copy[12];
        for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;

        // Try to make the move
        if (make_move(move) == 0) {
            continue; // Illegal move, board was restored automatically
        }

        legal_moves++;

        // Recursive Negamax call
        int score = -negamax(-beta, -alpha, depth - 1);

        // Restore board state
        for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        // Alpha-Beta Pruning
        if (score >= beta) {
            return beta; // Fail high (pruning)
        }
        if (score > alpha) {
            alpha = score; // Found a better move
        }
    }

    // Checkmate and Stalemate detection
    if (legal_moves == 0) {
        int king_square = (side == white) ? get_lsb_index(bitboards[K]) : get_lsb_index(bitboards[k]);
        if (is_square_attacked(king_square, side ^ 1)) {
            // Checkmate! Return a massively negative score, adjusted by depth to prefer faster mates
            return -49000 + (100 - depth);
        } else {
            // Stalemate! It's a draw.
            return 0;
        }
    }

    return alpha;
}

void search_position(int depth) {
    search_nodes = 0;
    best_move_found = 0;
    
    MoveList move_list;
    generate_moves(move_list, side);
    
    // Sort root moves!
    sort_moves(move_list);

    // Initial infinity values for alpha and beta
    int alpha = -50000;
    int beta = 50000;

    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];

        // Create local copies of the board state
        U64 bitboards_copy[12];
        for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;

        if (make_move(move) == 0) {
            continue; 
        }

        int score = -negamax(-beta, -alpha, depth - 1);

        // Restore board state
        for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        if (score > alpha) {
            alpha = score;
            best_move_found = move;
        }
    }

    // Format the move into an algebraic string (e.g., "e2e4") for the GUI
    if (best_move_found) {
        int source = GET_SOURCE(best_move_found);
        int target = GET_TARGET(best_move_found);
        int promoted = GET_PROMOTED(best_move_found);
        
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

        // Print UCI formatted output
        std::cout << "info depth " << depth << " score cp " << alpha << " nodes " << search_nodes << "\n";
        std::cout << "bestmove " << move_string << "\n";
    }
}