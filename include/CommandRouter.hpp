#ifndef COMMAND_ROUTER_HPP
#define COMMAND_ROUTER_HPP

#include <vector>
#include <string>
#include "IssueBoard.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// CommandRouter — CLI Flag Parser and Action Dispatcher
//
// Converts raw argc/argv inputs into validated IssueBoard method calls.
// All validation errors throw std::invalid_argument (exit code 1 in main).
// All logic errors propagate as std::runtime_error (exit code 2 in main).
//
// Supported flags:
//   --create-issue <title> <desc> <tag> [priority]
//   --poll-next
//   --list-json
//   --list-agents
//   --assign-issue <id> <agent_name>
//   --update-status <id> <STATUS>
//   --help
// ─────────────────────────────────────────────────────────────────────────────
class CommandRouter {
private:
    std::vector<std::string> args;

    // Validate that a required positional argument exists at index i.
    void requireArg(size_t index, const std::string& usage) const;

    // Parse a string as a positive integer; throws invalid_argument on failure.
    static int parseIntArg(const std::string& s, const std::string& fieldName);

public:
    CommandRouter(int argc, char* argv[]);

    // Route the parsed command to the appropriate IssueBoard action.
    void route(IssueBoard& board) const;

    // Print full CLI usage guide to stdout.
    static void printHelp();
};

#endif // COMMAND_ROUTER_HPP