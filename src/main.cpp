#include "IssueBoard.hpp"
#include "CommandRouter.hpp"
#include "Agent.hpp"
#include "MulticaWorkspace.hpp"
#include <iostream>
#include <unordered_map>

// ─────────────────────────────────────────────────────────────────────────────
// MicroMultica — Agent-Callable CLI Orchestration Engine
//
// Entry point: parses argv flags, routes to the IssueBoard state engine,
// and exits with a standardized exit code:
//   0 — Success
//   1 — Invalid argument / validation failure  (std::invalid_argument)
//   2 — Logic or system exception              (std::exception)
//
// Agent model:
//   A single "Coding Agent" is registered — representing any external AI agent
//   (e.g. one reading skill.md) that invokes this CLI to track and advance work.
//   The agent is identified by a string name and tracked for performance stats.
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

        // ── Agent Registration & Stats Restore ────────────────────────────
        // Register a single Coding Agent representing the external AI caller.
        // Historic performance stats are restored from disk to feed the
        // score-compounding algorithm on subsequent invocations.
        Agent codingAgent("Coding Agent");
        board.registerAgent(codingAgent);

        std::unordered_map<std::string, Agent> agentRestoreMap;
        agentRestoreMap[codingAgent.getName()] = codingAgent;
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