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
        // ── Workspace Initialization (Dependency Injection Source) ─────────
        // Create file-based workspace polymorphically using shared_ptr interface.
        std::shared_ptr<Workspace> workspace = std::make_shared<MulticaWorkspace>("multica_issues.dat", "multica_agents.dat");

        // ── IssueBoard Construction ───────────────────────────────────────
        // Inject workspace dependency into IssueBoard.
        IssueBoard board(workspace);

        // ── Agent Definition & Stats Restore ──────────────────────────────
        // Define simple Agent instances representing the active callers.
        Agent claudeAgent("Claude-3.5",     "auth");
        Agent cursorAgent("Cursor-Composer", "database");
        Agent geminiAgent("Gemini-Advanced", "frontend");

        // Build temporary map to restore historic success/total performance data
        std::unordered_map<std::string, Agent> agentRestoreMap;
        agentRestoreMap[claudeAgent.getName()] = claudeAgent;
        agentRestoreMap[cursorAgent.getName()] = cursorAgent;
        agentRestoreMap[geminiAgent.getName()] = geminiAgent;
        
        workspace->loadAgentStats(agentRestoreMap);

        // Register the updated agents by value into the board orchestrator
        board.registerAgent(agentRestoreMap[claudeAgent.getName()]);
        board.registerAgent(agentRestoreMap[cursorAgent.getName()]);
        board.registerAgent(agentRestoreMap[geminiAgent.getName()]);

        // ── CLI Routing ───────────────────────────────────────────────────
        CommandRouter router(argc, argv);
        router.route(board);

        return 0; // EXIT_SUCCESS

    } catch (const std::invalid_argument& e) {
        std::cerr << "{\"status\":\"error\","
                  << "\"type\":\"validation\","
                  << "\"message\":\"" << e.what() << "\"}\n";
        return 1; // EXIT_FAILURE — invalid argument

    } catch (const std::exception& e) {
        std::cerr << "{\"status\":\"error\","
                  << "\"type\":\"system_failure\","
                  << "\"message\":\"" << e.what() << "\"}\n";
        return 2; // EXIT_LOGIC_ERROR — operational exception
    }
}