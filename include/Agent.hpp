#ifndef AGENT_HPP
#define AGENT_HPP

#include <string>
#include "Issue.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Agent — Concrete Coding Agent
//
// Represents any external AI coding agent that interacts with this CLI.
// The agent reads skill.md and invokes multica commands — it is not defined
// here by type or subclass. This class only models its tracked identity and
// historic performance stats, which feed the score-compounding algorithm.
//
// Fields:
//   • name        : The agent's registered handle (e.g. "Coding Agent")
//   • successCount: Tasks it has completed without going BLOCKED
//   • totalCount  : Total tasks attempted (COMPLETED + BLOCKED)
//
// Task execution behavior (executeTask) simulates the real-world convention
// where any agent receiving a task description containing the word "CRASH"
// must halt and transition the issue to BLOCKED (as documented in skill.md).
// ─────────────────────────────────────────────────────────────────────────────
class Agent {
private:
    std::string name;
    int successCount;
    int totalCount;

public:
    Agent() : name(""), successCount(0), totalCount(0) {}

    explicit Agent(std::string n, int successes = 0, int total = 0)
        : name(std::move(n)), successCount(successes), totalCount(total) {}

    // ── Identity ───────────────────────────────────────────────────────────
    std::string getName() const { return name; }

    // ── Performance Tracking ───────────────────────────────────────────────
    int    getSuccessCount() const { return successCount; }
    int    getTotalCount()   const { return totalCount; }

    // Returns 1.0 (optimistic) if the agent has no task history yet
    double getSuccessRate() const {
        if (totalCount == 0) return 1.0;
        return static_cast<double>(successCount) / static_cast<double>(totalCount);
    }

    // Record the outcome of a completed task
    void recordResult(bool success) {
        totalCount++;
        if (success) successCount++;
    }

    // ── Score Compounding ──────────────────────────────────────────────────
    // Produces an assignment score relative to an issue.
    //
    // Formula:
    //   score = getSuccessRate() * 10.0    (0.0–10.0: weighted on task history)
    //         + (6 - issue.getPriority())  (1–5: high-priority issues score higher)
    //
    // Higher score → preferred candidate for the issue.
    double computeScore(const Issue& issue) const {
        double score = getSuccessRate() * 10.0;
        score += static_cast<double>(6 - issue.getPriority());
        return score;
    }

    // ── Task Execution Simulation ──────────────────────────────────────────
    // Models the behavior described in skill.md:
    // If a task description contains "CRASH", the agent must halt and the
    // issue transitions to BLOCKED — signaling human maintainer intervention.
    // All other tasks succeed by default.
    bool executeTask(const Issue& issue) const {
        if (issue.getDescription().find("CRASH") != std::string::npos) {
            return false; // Signal BLOCKED state
        }
        return true;
    }
};

#endif // AGENT_HPP