#ifndef ISSUE_BOARD_HPP
#define ISSUE_BOARD_HPP

#include <vector>
#include <unordered_map>
#include <optional>
#include <string>
#include "Issue.hpp"
#include "Agent.hpp"
#include "MulticaWorkspace.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// IssueBoard — Core State-Machine Orchestration Engine
//
// Acts as the central worker class of MicroMultica. It owns the issue
// collection, the agent registry, and all lifecycle mutation logic.
//
// Responsibilities:
//   • Maintain the ordered list of Issues with unique integer IDs
//   • Register and index Agent instances by name (value-stored, no polymorphism)
//   • Route issues through the ENQUEUED → CLAIMED → RUNNING → COMPLETED/BLOCKED
//     state machine using score-compounding deterministic assignment
//   • Emit structured JSON to stdout and errors to stderr
//   • Delegate all disk I/O to the injected MulticaWorkspace instance
// ─────────────────────────────────────────────────────────────────────────────
class IssueBoard {
private:
    std::vector<Issue>                    issues;
    std::unordered_map<std::string, Agent> agentRegistry;
    MulticaWorkspace                       workspace;

    // ── Internal helpers ───────────────────────────────────────────────────

    // Find an issue by ID; returns nullptr if not found.
    Issue* findIssueById(int id);
    const Issue* findIssueById(int id) const;

    // Score-compounding agent selection algorithm.
    // Iterates agentRegistry, calls agent.computeScore(issue), and returns
    // a pointer to the best-scoring Agent, or nullptr if the registry is empty.
    const Agent* findBestAgent(const Issue& issue) const;

    // Emit a JSON state-change event line to stdout.
    void emitStateChange(int issueId,
                         const std::string& newStatus,
                         const std::string& agent = "") const;

public:
    // Construct the board with a workspace instance (default paths apply).
    explicit IssueBoard(MulticaWorkspace ws = MulticaWorkspace{});

    // On destruction, persist current state to disk automatically.
    ~IssueBoard();

    // ── Agent Registry ─────────────────────────────────────────────────────

    // Register an agent by value; silently ignores agents with empty names.
    void registerAgent(Agent agent);

    // ── Issue Management ───────────────────────────────────────────────────

    // Create and append a new Issue. Validates that no field is empty and
    // priority is within [1, 5]. Emits JSON success or throws on validation fail.
    void addIssue(const std::string& title,
                  const std::string& desc,
                  const std::string& tag,
                  int priority = PRIORITY_MEDIUM);

    // Manually assign an issue to a named agent (must already be registered).
    // Transitions the issue to CLAIMED status.
    void assignIssue(int id, const std::string& agentName);

    // Force-update an issue's status from a string token (e.g. "COMPLETED").
    // Valid tokens: ENQUEUED, CLAIMED, RUNNING, COMPLETED, BLOCKED.
    void updateIssueStatus(int id, const std::string& statusStr);

    // ── State-Machine Lifecycle ────────────────────────────────────────────

    // Advance one issue by exactly one lifecycle step:
    //   ENQUEUED  → CLAIMED  (score-compounding assignment)
    //   CLAIMED   → RUNNING  (prepare for execution)
    //   RUNNING   → COMPLETED or BLOCKED (agent.executeTask())
    // Records agent performance stats on RUNNING→terminal transitions.
    // Emits an idle JSON message if no issue required a transition.
    void processNextLifecycleStep();

    // ── JSON Output ────────────────────────────────────────────────────────

    // Print all issues as a JSON array to stdout.
    void printBoardJSON() const;

    // Print all registered agents with their stats as a JSON array to stdout.
    void printAgentsJSON() const;
};

#endif // ISSUE_BOARD_HPP