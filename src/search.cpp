#include "search.h"
#include "evaluate.h"
#include "movegen.h"
#include "move.h"
#include "bitboard.h"
#include <iostream>

int best_move_found = 0;
long long search_nodes = 0;

int negamax(int alpha, int beta, int depth) {
    // Base Case: We reached the target depth, return the static evaluation
    if (depth == 0) {
        search_nodes++;
        return evaluate_position();
    }

    MoveList move_list;
    generate_moves(move_list, side);

    int legal_moves = 0;

    for (int i = 0; i < move_list.count; i++) {
        // Copy the board state
        U64 bitboards_copy[12];
        for (int j = 0; j < 12; j++) bitboards_copy[j] = bitboards[j];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;

        // Skip illegal moves that leave the king in check
        if (make_move(move_list.moves[i]) == 0) continue;
        
        legal_moves++;

        // Recursively evaluate the branch (flipping alpha/beta and the score)
        int score = -negamax(-beta, -alpha, depth - 1);

        // Restore the board state
        for (int j = 0; j < 12; j++) bitboards[j] = bitboards_copy[j];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        // Fail-Hard Beta Cutoff: The opponent has a better move elsewhere, stop searching here
        if (score >= beta) {
            return beta;
        }
        // Found a better move for the current player
        if (score > alpha) {
            alpha = score;
        }
    }

    if (legal_moves == 0) {
        int king_square = (side == white) ? get_lsb_index(bitboards[K]) : get_lsb_index(bitboards[k]);
        int attacking_side = side ^ 1; // The opponent
        
        if (is_square_attacked(king_square, attacking_side)) {
            // Checkmate! We add 'depth' to prefer checkmates that happen sooner rather than later
            return -49000 + depth; 
        } else {
            // Stalemate!
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

    // Initial infinity values for alpha and beta
    int alpha = -50000;
    int beta = 50000;

    for (int i = 0; i < move_list.count; i++) {
        // Copy state
        U64 bitboards_copy[12];
        for (int j = 0; j < 12; j++) bitboards_copy[j] = bitboards[j];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;

        if (make_move(move_list.moves[i]) == 0) continue;

        // Start Negamax
        int score = -negamax(-beta, -alpha, depth - 1);

        // Restore state
        for (int j = 0; j < 12; j++) bitboards[j] = bitboards_copy[j];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        // If we found a new best root move, save it
        if (score > alpha) {
            alpha = score;
            best_move_found = move_list.moves[i];
        }
    }

    // Convert the best 32-bit integer move back to an algebraic string for the GUI
    std::string best_move_str = "";
    if (best_move_found) {
        int src = GET_SOURCE(best_move_found);
        int tgt = GET_TARGET(best_move_found);
        int prom = GET_PROMOTED(best_move_found);
        
        best_move_str += (char)('a' + (src % 8));
        best_move_str += (char)('8' - (src / 8));
        best_move_str += (char)('a' + (tgt % 8));
        best_move_str += (char)('8' - (tgt / 8));
        
        if (prom) {
            if (prom == Q || prom == q) best_move_str += 'q';
            else if (prom == R || prom == r) best_move_str += 'r';
            else if (prom == B || prom == b) best_move_str += 'b';
            else if (prom == N || prom == n) best_move_str += 'n';
        }
    }

    // Output strictly in UCI format
    std::cout << "info depth " << depth << " score cp " << alpha << " nodes " << search_nodes << "\n";
    std::cout << "bestmove " << (best_move_found ? best_move_str : "(none)") << "\n";
}