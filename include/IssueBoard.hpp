#ifndef ISSUE_BOARD_HPP
#define ISSUE_BOARD_HPP

#include <vector>
#include <unordered_map>
#include <optional>
#include <string>
#include <memory>
#include "Issue.hpp"
#include "Agent.hpp"
#include "MulticaWorkspace.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// IssueBoard — Orchestration Layer
//
// Manages issue lifecycles and agent allocations. Uses Dependency Injection (DI)
// to receive its Workspace component, keeping storage decoupled.
// ─────────────────────────────────────────────────────────────────────────────
class IssueBoard {
private:
    std::vector<Issue>                     issues;
    std::unordered_map<std::string, Agent> agentRegistry; // Stored by value
    std::shared_ptr<Workspace>             workspace;     // Injected interface

    // ── Internal helpers ───────────────────────────────────────────────────
    Issue* findIssueById(int id);
    const Issue* findIssueById(int id) const;

    // Returns a pointer to the best-scoring agent, or nullptr if none registered.
    const Agent* findBestAgent(const Issue& issue) const;

    void emitStateChange(int issueId,
                         const std::string& newStatus,
                         const std::string& agent = "") const;

public:
    // Inject the Workspace dependency via the constructor
    explicit IssueBoard(std::shared_ptr<Workspace> ws = nullptr);

    // RAII destructor: saves state via the injected workspace
    ~IssueBoard();

    // ── Agent Registry ─────────────────────────────────────────────────────
    void registerAgent(Agent agent);

    // ── Issue Management ───────────────────────────────────────────────────
    void addIssue(const std::string& title,
                  const std::string& desc,
                  const std::string& tag,
                  int priority = PRIORITY_MEDIUM);

    void assignIssue(int id, const std::string& agentName);
    void updateIssueStatus(int id, const std::string& statusStr);

    // ── State-Machine Lifecycle ────────────────────────────────────────────
    void processNextLifecycleStep();

    // ── JSON Output ────────────────────────────────────────────────────────
    void printBoardJSON() const;
    void printAgentsJSON() const;
};

#endif // ISSUE_BOARD_HPP