#include <iostream>
#include <chrono>
#include "bitboard.h"
#include "movegen.h"
#include "move.h"
#include "evaluate.h"
#include "search.h" // Include our new search header

// Global node counter to track total positions evaluated
long long nodes = 0;

// Coordinate mapping for clean console output
const char* square_to_coord[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

static inline void perft_driver(int depth) {
    // Base case: If we reach depth 0, we've found a leaf node
    if (depth == 0) {
        nodes++;
        return;
    }

    MoveList move_list;
    generate_moves(move_list, side);

    for (int i = 0; i < move_list.count; i++) {
        // Copy-Make approach: Save the current global board state
        U64 bitboards_copy[12];
        for (int j = 0; j < 12; j++) bitboards_copy[j] = bitboards[j];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;

        // Make the move. If it's illegal (leaves King in check), skip it.
        if (make_move(move_list.moves[i]) == 0) continue;

        // Recursively evaluate the next depth
        perft_driver(depth - 1);

        // Restore the global board state
        for (int j = 0; j < 12; j++) bitboards[j] = bitboards_copy[j];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;
    }
}

void perft_test(int depth) {
    std::cout << "\nPerformance Test (Perft) started at Depth " << depth << "...\n\n";
    nodes = 0;
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    MoveList move_list;
    generate_moves(move_list, side);

    // Loop through root moves to display individual move node counts
    for (int i = 0; i < move_list.count; i++) {
        // Save state
        U64 bitboards_copy[12];
        for (int j = 0; j < 12; j++) bitboards_copy[j] = bitboards[j];
        int side_copy = side;
        int enpassant_copy = enpassant;
        int castle_copy = castle;

        int move = move_list.moves[i];

        if (make_move(move) == 0) continue;

        // Track how many nodes this specific move branch generates
        long long cumulative_nodes = nodes;
        perft_driver(depth - 1);
        long long old_nodes = nodes - cumulative_nodes;

        // Restore state
        for (int j = 0; j < 12; j++) bitboards[j] = bitboards_copy[j];
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        std::cout << "Move: " 
                  << square_to_coord[GET_SOURCE(move)] 
                  << square_to_coord[GET_TARGET(move)] 
                  << "  Nodes: " << old_nodes << "\n";
    }

    // Stop timing
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    // Output final results
    std::cout << "\nTest Complete!\n";
    std::cout << "Depth: " << depth << "\n";
    std::cout << "Total Nodes generated: " << nodes << "\n";
    std::cout << "Time elapsed: " << duration.count() << " seconds\n";
    
    // Calculate and print Nodes Per Second (NPS)
    long long nps = (long long)(nodes / duration.count());
    std::cout << "Speed: " << nps << " NPS\n\n";
}

int main() {
    std::cout << "Project A Engine Initialized\n";
    init_leapers(); // Initialize lookup tables
    
    // Set up a classic Mate in 1 puzzle (White to move)
    // The Black King is trapped on a8. White Queen is on h7. White King is on a6.
    std::string fen = "k7/7Q/K7/8/8/8/8/8 w - - 0 1";
    parse_fen(fen);
    
    std::cout << "\nPosition: Mate in 1 for White\n";
    print_board();
    
    // Search the position to depth 4
    std::cout << "Thinking...\n";
    search_position(4);
    
    return 0;
}