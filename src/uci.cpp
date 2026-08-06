#include "uci.h"
#include "bitboard.h"
#include "movegen.h"
#include "search.h"
#include "tt.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstdio> 

int parse_move(std::string move_string) {
    MoveList move_list;
    generate_moves(move_list, side);

    int source_file = move_string[0] - 'a';
    int source_rank = 8 - (move_string[1] - '0');
    int target_file = move_string[2] - 'a';
    int target_rank = 8 - (move_string[3] - '0');
    
    int source = source_rank * 8 + source_file;
    int target = target_rank * 8 + target_file;

    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];
        if (source == GET_SOURCE(move) && target == GET_TARGET(move)) {
            int promoted_piece = GET_PROMOTED(move);
            if (promoted_piece) {
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q') return move;
                if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r') return move;
                if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b') return move;
                if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n') return move;
                continue; 
            }
            return move; 
        }
    }
    return 0; 
}

void uci_loop() {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    std::string line;
    std::cout << "Project A Engine - UCI Mode Started\n";

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

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
            clear_tt(); 
            parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        } 
        else if (token == "position") {
            iss >> token;
            if (token == "startpos") {
                parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
                iss >> token; 
            } else if (token == "fen") {
                std::string fen;
                while (iss >> token && token != "moves") {
                    fen += token + " ";
                }
                parse_fen(fen);
            }
            
            while (iss >> token) {
                int move = parse_move(token);
                if (move == 0) break;
                make_move(move);
            }
        } 
        else if (token == "go") {
            int depth = -1; 
            int wtime = -1, btime = -1, movetime = -1;
            int winc = 0, binc = 0, movestogo = 40; // Default to 40 moves remaining
            
            while (iss >> token) {
                if (token == "depth") iss >> depth;
                else if (token == "wtime") iss >> wtime;
                else if (token == "btime") iss >> btime;
                else if (token == "winc") iss >> winc;
                else if (token == "binc") iss >> binc;
                else if (token == "movestogo") iss >> movestogo;
                else if (token == "movetime") iss >> movetime;
            }
            
            allocated_time = -1; 
            
            // 1. Fixed time per move
            if (movetime != -1) {
                allocated_time = movetime - 50; // 50ms safety margin
                if (depth == -1) depth = 64; 
            } 
            // 2. Tournament time control
            else if (wtime != -1 && btime != -1) {
                int time_left = (side == white) ? wtime : btime;
                int increment = (side == white) ? winc : binc;
                
                if (movestogo == 0) movestogo = 40; // Default to 40 if GUI omits it
                
                // Standard Allocation
                allocated_time = (time_left / movestogo) + (increment * 3 / 4) - 50; 
                
                // Panic State: If under 5 seconds, move lightning fast!
                if (time_left < 5000) {
                    allocated_time = time_left / 15;
                }
                
                if (allocated_time < 50) allocated_time = 50; 
                if (depth == -1) depth = 64; 
            } 
            // 3. Fallback if user just types "go"
            else if (depth == -1) {
                depth = 5; 
            }
            
            abort_search = false;
            start_time = std::chrono::steady_clock::now();
            search_position(depth);
        }
    }
}