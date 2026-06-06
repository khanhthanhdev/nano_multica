#ifndef AGENT_HPP
#define AGENT_HPP

#include <string>
#include <algorithm>
#include <stdexcept>
#include "Issue.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Agent — Simple Concrete Agent (represents a standard user profile)
//
// Encapsulates identity and historic performance data. Task execution rules and
// score calculation behaviors are dynamically determined by fields like the
// agent's specialty tag, rather than class inheritance.
// ─────────────────────────────────────────────────────────────────────────────
class Agent {
private:
    std::string name;
    std::string specialtyTag;
    int successCount = 0;
    int totalCount = 0;

public:
    Agent() : name(""), specialtyTag(""), successCount(0), totalCount(0) {}

    explicit Agent(std::string n, std::string tag = "", int successes = 0, int total = 0)
        : name(std::move(n)), specialtyTag(std::move(tag)),
          successCount(successes), totalCount(total) {}

    // ── Identity Accessors ─────────────────────────────────────────────────
    std::string getName()      const { return name; }
    std::string getSpecialty() const { return specialtyTag; }

    // ── Performance Accessors ──────────────────────────────────────────────
    int    getSuccessCount() const { return successCount; }
    int    getTotalCount()   const { return totalCount; }

    double getSuccessRate() const {
        if (totalCount == 0) return 1.0;
        return static_cast<double>(successCount) / static_cast<double>(totalCount);
    }

    void recordResult(bool success) {
        totalCount++;
        if (success) successCount++;
    }

    // ── Score Compounding ──────────────────────────────────────────────────
    double computeScore(const Issue& issue) const {
        double score = 0.0;
        // Specialty match bonus
        if (specialtyTag == issue.getTag()) {
            score += 10.0;
        }
        // Success rate performance factor
        score += getSuccessRate() * 5.0;
        // Priority weight
        score += static_cast<double>(6 - issue.getPriority());
        
        // Dynamic frontend escalation bonus (matches original Gemini subclass logic)
        if (specialtyTag == "frontend" && issue.getPriority() <= 2) {
            score += 3.0;
        }
        return score;
    }

    // ── Dynamic Simulated Task Execution ───────────────────────────────────
    bool executeTask(const Issue& issue) const {
        // Auth/Security rules: fail on simulated crash conditions
        if (specialtyTag == "auth" && issue.getDescription().find("CRASH") != std::string::npos) {
            return false;
        }
        // Database rules: fail if the issue title is empty (malformed task context)
        if (specialtyTag == "database" && issue.getTitle().empty()) {
            return false;
        }
        // Frontend/UI rules: fail on unverified critical requirements
        if (specialtyTag == "frontend" && issue.getPriority() == 1 && issue.getDescription().find("UNVERIFIED") != std::string::npos) {
            return false;
        }
        // Research rules: fail if input size exceeds context window limit (500 chars)
        if (specialtyTag == "research" && issue.getDescription().length() > 500) {
            return false;
        }
        return true;
    }
};

#endif // AGENT_HPP