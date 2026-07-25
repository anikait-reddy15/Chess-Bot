#include "uci.h"
#include "bitboard.h"
#include "movegen.h"
#include "search.h"
#include <iostream>
#include <sstream>
#include <cstdio> // Include cstdio for setvbuf

int parse_move(std::string move_string) {
    // 1. Generate all pseudo-legal moves for the current position
    MoveList move_list;
    generate_moves(move_list, side);

    // 2. Parse source and target squares from the string
    int source_file = move_string[0] - 'a';
    int source_rank = 8 - (move_string[1] - '0');
    int target_file = move_string[2] - 'a';
    int target_rank = 8 - (move_string[3] - '0');
    
    int source_square = source_rank * 8 + source_file;
    int target_square = target_rank * 8 + target_file;

    // 3. Search our move list for a match
    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];
        
        if (GET_SOURCE(move) == source_square && GET_TARGET(move) == target_square) {
            int promoted = GET_PROMOTED(move);
            
            // If it's a promotion move, ensure the promoted piece matches the string (e.g., "e7e8q")
            if (promoted) {
                if ((promoted == Q || promoted == q) && move_string[4] == 'q') return move;
                if ((promoted == R || promoted == r) && move_string[4] == 'r') return move;
                if ((promoted == B || promoted == b) && move_string[4] == 'b') return move;
                if ((promoted == N || promoted == n) && move_string[4] == 'n') return move;
                continue; // Wrong promotion piece, keep looking
            }
            
            // Return the full 32-bit integer!
            return move;
        }
    }
    
    return 0; // Move not found / illegal
}

void uci_loop() {
    // Disable buffering for instant GUI communication
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    std::string line;
    std::cout << "Project A Engine - UCI Mode Started\n";

    while (std::getline(std::cin, line)) {
        std::istringstream is(line);
        std::string token;
        is >> token;

        if (token == "quit") {
            break;
        } 
        else if (token == "uci") {
            std::cout << "id name Project A\n";
            std::cout << "id author You\n";
            std::cout << "uciok\n";
        } 
        else if (token == "isready") {
            std::cout << "readyok\n";
        } 
        else if (token == "ucinewgame") {
            parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        } 
        else if (token == "position") {
            std::string position_type;
            is >> position_type;
            
            if (position_type == "startpos") {
                parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
                is >> token; // Consume the "moves" token if it exists
            } else if (position_type == "fen") {
                std::string fen = "";
                for (int i = 0; i < 6; i++) {
                    is >> token;
                    fen += token + " ";
                }
                parse_fen(fen);
                is >> token; // Consume the "moves" token
            }
            
            // Play all moves sent by the GUI on our internal board
            while (is >> token) {
                int parsed_move = parse_move(token);
                if (parsed_move) {
                    make_move(parsed_move);
                }
            }
        } 
        else if (token == "go") {
            int depth = -1;
            int time_remaining = -1;
            allocated_time = -1;
            abort_search = false;

            std::string arg;
            while (is >> arg) {
                if (arg == "depth") {
                    is >> depth;
                } else if (arg == "wtime" && side == white) {
                    is >> time_remaining;
                } else if (arg == "btime" && side == black) {
                    is >> time_remaining;
                } else if (arg == "movetime") {
                    is >> allocated_time;
                }
            }

            // If the GUI sent time remaining (e.g. wtime 60000), allocate ~1/30th of it for this move
            if (time_remaining != -1 && allocated_time == -1) {
                allocated_time = time_remaining / 30;
            }

            // If no depth was provided, search extremely deep (we will rely on the clock to stop us)
            if (depth == -1) {
                depth = 64; 
            }

            // Start the clock and trigger the search!
            start_time = std::chrono::steady_clock::now();
            search_position(depth);
        }
    }
}