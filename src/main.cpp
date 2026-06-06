#include "IssueBoard.hpp"
#include "CommandRouter.hpp"
#include "Agent.hpp"
#include "MulticaWorkspace.hpp"
#include <iostream>
#include <memory>
#include <unordered_map>

// ─────────────────────────────────────────────────────────────────────────────
// MicroMultica — Agent-Callable CLI Orchestration Engine
//
// Entry point: parses argv flags, routes to the IssueBoard state engine,
// and exits with a standardized exit code:
//   0 — Success
//   1 — Invalid argument / validation failure  (std::invalid_argument)
//   2 — Logic or system exception              (std::exception)
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    try {
        // ── Workspace Initialization ──────────────────────────────────────
        // MulticaWorkspace abstracts all disk I/O.
        // Default paths: multica_issues.dat & multica_agents.dat
        MulticaWorkspace workspace("multica_issues.dat", "multica_agents.dat");

        // ── IssueBoard Construction ───────────────────────────────────────
        // Constructor loads persisted issue records from disk via the workspace.
        // Destructor will save all state back to disk on scope exit (RAII commit).
        IssueBoard board(workspace);

        // ── Agent Registration ────────────────────────────────────────────
        // Dynamically allocate polymorphic agents via memory-safe shared_ptr
        // wrappers. Each agent declares a specialty tag that maps to Issue tags,
        // enabling score-compounding assignment in the state machine.
        auto claudeAgent = std::make_shared<ClaudeAgent>("Claude-3.5",     "auth");
        auto cursorAgent = std::make_shared<CursorAgent>("Cursor-Composer", "database");
        auto geminiAgent = std::make_shared<GeminiAgent>("Gemini-Advanced", "frontend");

        board.registerAgent(claudeAgent);
        board.registerAgent(cursorAgent);
        board.registerAgent(geminiAgent);

        // ── Restore Historic Agent Performance Stats ──────────────────────
        // After agents are registered, load their persisted success/total counts
        // from the agents file. This feeds into the score-compounding algorithm
        // so agents improve or degrade in priority based on historic performance.
        //
        // Build a temporary registry map pointing to the live agent objects
        // so the workspace can match by name and replay recorded results.
        std::unordered_map<std::string, std::shared_ptr<Agent>> agentRestoreMap;
        agentRestoreMap[claudeAgent->getName()] = claudeAgent;
        agentRestoreMap[cursorAgent->getName()] = cursorAgent;
        agentRestoreMap[geminiAgent->getName()] = geminiAgent;
        workspace.loadAgentStats(agentRestoreMap);

        // ── CLI Routing ───────────────────────────────────────────────────
        // CommandRouter validates argv and dispatches to the appropriate
        // IssueBoard method. Throws std::invalid_argument on validation
        // failures and std::runtime_error on system exceptions.
        CommandRouter router(argc, argv);
        router.route(board);

        // board destructor auto-commits final state to disk at scope exit
        return 0; // EXIT_SUCCESS

    } catch (const std::invalid_argument& e) {
        // Validation errors: missing fields, unknown flags, bad IDs, etc.
        std::cerr << "{\"status\":\"error\","
                  << "\"type\":\"validation\","
                  << "\"message\":\"" << e.what() << "\"}\n";
        return 1; // EXIT_FAILURE — invalid argument

    } catch (const std::exception& e) {
        // System or logic exceptions: file I/O failures, state inconsistencies
        std::cerr << "{\"status\":\"error\","
                  << "\"type\":\"system_failure\","
                  << "\"message\":\"" << e.what() << "\"}\n";
        return 2; // EXIT_LOGIC_ERROR — operational exception
    }
}