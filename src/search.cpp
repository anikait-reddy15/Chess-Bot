#include "search.h"
#include "bitboard.h"
#include "movegen.h"
#include "evaluate.h"
#include "tt.h"
#include "zobrist.h" // Add this to access the Zobrist keys for NMP
#include <iostream>
#include <chrono>

bool abort_search = false;
long long allocated_time = -1;
std::chrono::time_point<std::chrono::steady_clock> start_time;

int search_nodes = 0;
int best_move_found = 0;

long long get_time_ms() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
}

void check_time() {
    if (allocated_time > 0 && get_time_ms() >= allocated_time) {
        abort_search = true;
    }
}

int score_move(int move, int hash_move) {
    if (move == hash_move) return 30000;

    int score = 0;
    int promoted = GET_PROMOTED(move);
    if (promoted) score += 10000 + piece_weights[promoted]; 
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

void sort_moves(MoveList &move_list, int hash_move) {
    int scores[256];
    for (int i = 0; i < move_list.count; i++) scores[i] = score_move(move_list.moves[i], hash_move);
    for (int i = 0; i < move_list.count; i++) {
        int max_idx = i;
        for (int j = i + 1; j < move_list.count; j++) {
            if (scores[j] > scores[max_idx]) max_idx = j;
        }
        int temp_score = scores[i];
        scores[i] = scores[max_idx];
        scores[max_idx] = temp_score;
        int temp_move = move_list.moves[i];
        move_list.moves[i] = move_list.moves[max_idx];
        move_list.moves[max_idx] = temp_move;
    }
}

int quiescence(int alpha, int beta) {
    if ((search_nodes & 2047) == 0) check_time();
    if (abort_search) return 0;

    search_nodes++;
    int evaluation = evaluate_position();
    if (evaluation >= beta) return beta;
    if (evaluation > alpha) alpha = evaluation;
    
    MoveList move_list;
    generate_moves(move_list, side);
    sort_moves(move_list, 0); 
    
    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];
        if (!GET_CAPTURE(move)) continue;
        
        U64 bitboards_copy[12];
        for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;
        U64 hash_key_copy = hash_key;
        
        if (make_move(move) == 0) continue;
        int score = -quiescence(-beta, -alpha);
        
        for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;
        hash_key = hash_key_copy;
        
        if (abort_search) return 0;
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// Updated signature to track if we are allowed to make a null move
int negamax(int alpha, int beta, int depth, bool allow_null) {
    if ((search_nodes & 2047) == 0) check_time();
    if (abort_search) return 0;

    int hash_move = 0;
    int tt_score = read_tt(alpha, beta, depth, hash_move);
    if (tt_score != NO_HASH_ENTRY) return tt_score;

    if (depth == 0) return quiescence(alpha, beta);
    search_nodes++;

    int king_square = (side == white) ? get_lsb_index(bitboards[K]) : get_lsb_index(bitboards[k]);
    int in_check = is_square_attacked(king_square, side ^ 1);

    // --- NULL MOVE PRUNING ---
    // If depth is high enough, we are not in check, and we haven't just done a null move
    if (allow_null && depth >= 3 && !in_check) {
        int enpassant_copy = enpassant;
        U64 hash_key_copy = hash_key;

        // "Pass" the turn
        side ^= 1;
        hash_key ^= side_key;
        if (enpassant != -1) {
            hash_key ^= enpassant_keys[enpassant];
            enpassant = -1;
        }

        // Search with a reduced depth (R = 2) and forbid back-to-horrible-back null moves
        int score = -negamax(-beta, -beta + 1, depth - 1 - 2, false);

        // Restore the board
        side ^= 1;
        enpassant = enpassant_copy;
        hash_key = hash_key_copy;

        if (abort_search) return 0;

        // If the score is STILL too good, prune the branch!
        if (score >= beta) return beta;
    }
    // -------------------------

    MoveList move_list;
    generate_moves(move_list, side);
    sort_moves(move_list, hash_move);

    int legal_moves = 0;
    int alpha_orig = alpha; 
    int best_move_this_node = 0;

    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];
        
        U64 bitboards_copy[12];
        for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;
        U64 hash_key_copy = hash_key;

        if (make_move(move) == 0) continue;
        legal_moves++;
        
        // Normal recursive search, passing 'true' to allow null moves again
        int score = -negamax(-beta, -alpha, depth - 1, true);

        for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;
        hash_key = hash_key_copy;

        if (abort_search) return 0;
        
        if (score >= beta) {
            write_tt(depth, beta, HASH_BETA, move);
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            best_move_this_node = move; 
        }
    }

    if (legal_moves == 0) {
        if (in_check) return -49000 + (100 - depth);
        else return 0;
    }

    int flag = (alpha > alpha_orig) ? HASH_EXACT : HASH_ALPHA;
    write_tt(depth, alpha, flag, best_move_this_node);
    
    return alpha;
}

void search_position(int max_depth) {
    int best_move_overall = 0;
    
    for (int current_depth = 1; current_depth <= max_depth; current_depth++) {
        search_nodes = 0;
        best_move_found = 0;
        
        MoveList move_list;
        generate_moves(move_list, side);
        
        int hash_move = 0;
        read_tt(-50000, 50000, current_depth, hash_move);
        sort_moves(move_list, hash_move);

        int alpha = -50000;
        int beta = 50000;

        for (int i = 0; i < move_list.count; i++) {
            int move = move_list.moves[i];

            U64 bitboards_copy[12];
            for (int b = 0; b < 12; b++) bitboards_copy[b] = bitboards[b];
            int side_copy = side;
            int enpassant_copy = enpassant;
            int castle_copy = castle;
            U64 hash_key_copy = hash_key;

            if (make_move(move) == 0) continue; 
            
            // Start the root search allowing null moves on the branches
            int score = -negamax(-beta, -alpha, current_depth - 1, true);

            for (int b = 0; b < 12; b++) bitboards[b] = bitboards_copy[b];
            side = side_copy;
            enpassant = enpassant_copy;
            castle = castle_copy;
            hash_key = hash_key_copy;

            if (abort_search) break;

            if (score > alpha) {
                alpha = score;
                best_move_found = move;
            }
        }
        
        if (abort_search) break; 
        
        if (best_move_found != 0) {
            best_move_overall = best_move_found;
        }

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
            std::cout << "info depth " << current_depth << " score cp " << alpha << " nodes " << search_nodes << " time " << get_time_ms() << "\n";
        }
    }

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