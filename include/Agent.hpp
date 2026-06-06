#ifndef AGENT_HPP
#define AGENT_HPP

#include <string>
#include <algorithm>
#include <stdexcept>
#include "Issue.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Agent — Abstract Base Class
//
// Represents an AI coding agent profile. Provides:
//   • Identity  : name + specialtyTag (maps to Issue tags like "auth", "database")
//   • Performance: historic success/failure counters used for score compounding
//   • Polymorphism: pure-virtual executeTask() overridden by each concrete agent
// ─────────────────────────────────────────────────────────────────────────────
class Agent {
protected:
    std::string name;
    std::string specialtyTag;

    // ── Historic Performance Counters ──────────────────────────────────────
    int successCount;   // Number of tasks completed successfully
    int totalCount;     // Total tasks attempted

public:
    Agent(std::string n, std::string tag, int successes = 0, int total = 0)
        : name(std::move(n)), specialtyTag(std::move(tag)),
          successCount(successes), totalCount(total) {}

    virtual ~Agent() = default;

    // ── Identity Accessors ─────────────────────────────────────────────────
    std::string getName()      const { return name; }
    std::string getSpecialty() const { return specialtyTag; }

    // ── Performance Accessors ──────────────────────────────────────────────
    int    getSuccessCount() const { return successCount; }
    int    getTotalCount()   const { return totalCount; }

    // Historic success rate: returns 1.0 (perfect) if no tasks yet (optimistic default)
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
    // Compound score for assignment eligibility relative to a given issue.
    //
    // Formula:
    //   score  = specialty_match  ?  10  :  0        (tag affinity)
    //          + getSuccessRate() * 5.0               (0.0 – 5.0 based on history)
    //          + (6 - issue.getPriority())            (high-priority issues score higher)
    //
    // Higher score → preferred assignment candidate.
    virtual double computeScore(const Issue& issue) const {
        double score = 0.0;
        if (specialtyTag == issue.getTag()) score += 10.0;
        score += getSuccessRate() * 5.0;
        score += static_cast<double>(6 - issue.getPriority());
        return score;
    }

    // ── Polymorphic Task Execution ─────────────────────────────────────────
    // Returns true on simulated success, false triggers BLOCKED state.
    virtual bool executeTask(const Issue& issue) = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// ClaudeAgent — Derived Polymorphic Agent Class (Specialty: "auth")
//
// Simulates Claude engine processing behaviors. Fails if the issue description
// contains the keyword "CRASH" (maps to a Blocker state trigger).
// ─────────────────────────────────────────────────────────────────────────────
class ClaudeAgent : public Agent {
public:
    // Inherit base constructor
    using Agent::Agent;

    bool executeTask(const Issue& issue) override {
        // Simulate Claude processing: CRASH keyword triggers failure
        if (issue.getDescription().find("CRASH") != std::string::npos) {
            return false; // Task execution failure → BLOCKED state
        }
        return true;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// CursorAgent — Derived Polymorphic Agent Class (Specialty: "database")
//
// Simulates Cursor contextual generation structures. Fails on empty title
// (representing a malformed or incomplete task specification).
// ─────────────────────────────────────────────────────────────────────────────
class CursorAgent : public Agent {
public:
    // Inherit base constructor
    using Agent::Agent;

    bool executeTask(const Issue& issue) override {
        // Simulate Cursor processing: empty title = malformed task
        if (issue.getTitle().empty()) {
            return false;
        }
        return true;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// GeminiAgent — Derived Polymorphic Agent Class (Specialty: "frontend")
//
// Simulates Gemini multi-modal reasoning. Fails if issue priority is CRITICAL
// and no assignee is set (representing an escalation guard).
// ─────────────────────────────────────────────────────────────────────────────
class GeminiAgent : public Agent {
public:
    using Agent::Agent;

    // Override score: Gemini gets a bonus for high-priority issues
    double computeScore(const Issue& issue) const override {
        double score = Agent::computeScore(issue);
        if (issue.getPriority() <= 2) score += 3.0; // Bonus for CRITICAL/HIGH priority
        return score;
    }

    bool executeTask(const Issue& issue) override {
        // Gemini escalation guard: critical issues with no real assignee context fail
        if (issue.getPriority() == 1 && issue.getDescription().find("UNVERIFIED") != std::string::npos) {
            return false;
        }
        return true;
    }
};

#endif // AGENT_HPP