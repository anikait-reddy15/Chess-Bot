#ifndef UCI_H
#define UCI_H

#include <string>

// Parses a string move (e.g., "e2e4") into our 32-bit integer move
int parse_move(std::string move_string);

// The main infinite loop that listens to GUI commands
void uci_loop();

#endif