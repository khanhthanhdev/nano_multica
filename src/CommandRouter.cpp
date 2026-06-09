#include "CommandRouter.hpp"
#include <iostream>
#include <stdexcept>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// CommandRouter — Implementation
// ─────────────────────────────────────────────────────────────────────────────

CommandRouter::CommandRouter(int argc, char* argv[]) {
    // Safe conversion of raw C-string argv array into an STL vector
    args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
}

// ── Internal Helpers ───────────────────────────────────────────────────────

void CommandRouter::requireArg(size_t index, const std::string& usage) const {
    if (index >= args.size() || args[index].empty()) {
        throw std::invalid_argument("Missing required argument. Usage: " + usage);
    }
}

int CommandRouter::parseIntArg(const std::string& s, const std::string& fieldName) {
    try {
        size_t pos = 0;
        int value = std::stoi(s, &pos);
        if (pos != s.size()) {
            throw std::invalid_argument("Non-numeric characters detected.");
        }
        return value;
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "Invalid integer value for <" + fieldName + ">: \"" + s + "\"");
    }
}

// ── Help Output ────────────────────────────────────────────────────────────

void CommandRouter::printHelp() {
    std::cout <<
        "┌─────────────────────────────────────────────────────────────┐\n"
        "│         NanoMultica CLI — Agent-Callable Orchestrator      │\n"
        "└─────────────────────────────────────────────────────────────┘\n"
        "\n"
        "Usage: ./multica <flag> [arguments]\n"
        "\n"
        "  --create-issue <title> <desc> <tag> [priority]\n"
        "      Create a new issue. Priority is 1 (CRITICAL) to 5 (MINIMAL);\n"
        "      defaults to 3 (MEDIUM) if omitted.\n"
        "      Example: ./multica --create-issue \"Fix Auth\" \"OAuth broken\" auth 2\n"
        "\n"
        "  --poll-next\n"
        "      Advance one issue by a single lifecycle step:\n"
        "      ENQUEUED → CLAIMED (score-based assignment)\n"
        "      CLAIMED  → RUNNING\n"
        "      RUNNING  → COMPLETED | BLOCKED (via agent execution)\n"
        "\n"
        "  --list-json\n"
        "      Print all issues as a JSON array to stdout.\n"
        "\n"
        "  --list-agents\n"
        "      Print all registered agents with performance stats as JSON.\n"
        "\n"
        "  --assign-issue <id> <agent_name>\n"
        "      Manually assign issue <id> to a registered agent.\n"
        "      Example: ./multica --assign-issue 3 \"Claude-3.5\"\n"
        "\n"
        "  --update-status <id> <STATUS>\n"
        "      Force-update an issue's status. Valid STATUS values:\n"
        "      ENQUEUED | CLAIMED | RUNNING | COMPLETED | BLOCKED\n"
        "      Example: ./multica --update-status 2 BLOCKED\n"
        "\n"
        "  --help\n"
        "      Print this usage guide.\n"
        "\n"
        "Exit Codes:\n"
        "  0 — Success\n"
        "  1 — Invalid argument or validation failure\n"
        "  2 — Logic or system exception\n"
        "\n";
}

// ── Route Dispatch ─────────────────────────────────────────────────────────

void CommandRouter::route(IssueBoard& board) const {
    // Enforce minimum argc: binary name + at least one flag
    if (args.size() < 2) {
        throw std::invalid_argument(
            "No command flag provided. Run './multica --help' for usage details.");
    }

    const std::string& cmd = args[1];

    // ── --create-issue <title> <desc> <tag> [priority] ──────────────────
    if (cmd == "--create-issue") {
        requireArg(2, "--create-issue <title> <desc> <tag> [priority]");
        requireArg(3, "--create-issue <title> <desc> <tag> [priority]");
        requireArg(4, "--create-issue <title> <desc> <tag> [priority]");

        int priority = PRIORITY_MEDIUM; // Default priority
        if (args.size() >= 6) {
            priority = parseIntArg(args[5], "priority");
            if (priority < 1 || priority > 5) {
                throw std::invalid_argument(
                    "Priority must be 1 (CRITICAL) to 5 (MINIMAL). Got: " +
                    std::to_string(priority));
            }
        }
        board.addIssue(args[2], args[3], args[4], priority);

    // ── --poll-next ──────────────────────────────────────────────────────
    } else if (cmd == "--poll-next") {
        board.processNextLifecycleStep();

    // ── --list-json ──────────────────────────────────────────────────────
    } else if (cmd == "--list-json") {
        board.printBoardJSON();

    // ── --list-agents ────────────────────────────────────────────────────
    } else if (cmd == "--list-agents") {
        board.printAgentsJSON();

    // ── --assign-issue <id> <agent_name> ────────────────────────────────
    } else if (cmd == "--assign-issue") {
        requireArg(2, "--assign-issue <id> <agent_name>");
        requireArg(3, "--assign-issue <id> <agent_name>");

        int id = parseIntArg(args[2], "id");
        board.assignIssue(id, args[3]);

    // ── --update-status <id> <STATUS> ────────────────────────────────────
    } else if (cmd == "--update-status") {
        requireArg(2, "--update-status <id> <STATUS>");
        requireArg(3, "--update-status <id> <STATUS>");

        int id = parseIntArg(args[2], "id");
        board.updateIssueStatus(id, args[3]);

    // ── --help ───────────────────────────────────────────────────────────
    } else if (cmd == "--help") {
        printHelp();

    // ── Unknown flag ─────────────────────────────────────────────────────
    } else {
        throw std::invalid_argument(
            "Unknown command flag: \"" + cmd + "\". "
            "Run './multica --help' for available options.");
    }
}