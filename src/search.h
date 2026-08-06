#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>

// Time management variables
extern bool abort_search;
extern long long allocated_time;
extern std::chrono::time_point<std::chrono::steady_clock> start_time;

// Core search function to find the best move at a given depth
void search_position(int depth);

// Helper function to get current time in milliseconds
long long get_time_ms();

// Arrays to store killer moves for move ordering
extern int killer_moves[2][64];

#endif