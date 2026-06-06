#include "IssueBoard.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <sstream>

// ─────────────────────────────────────────────────────────────────────────────
// IssueBoard — Implementation
// ─────────────────────────────────────────────────────────────────────────────

IssueBoard::IssueBoard(MulticaWorkspace ws) : workspace(std::move(ws)) {
    // Load persisted state from disk on construction
    workspace.load(issues, agentRegistry);
}

IssueBoard::~IssueBoard() {
    // Persist current state to disk on destruction (RAII commit pattern)
    workspace.save(issues, agentRegistry);
}

// ── Internal Helpers ───────────────────────────────────────────────────────

Issue* IssueBoard::findIssueById(int id) {
    for (auto& issue : issues) {
        if (issue.getId() == id) return &issue;
    }
    return nullptr;
}

const Issue* IssueBoard::findIssueById(int id) const {
    for (const auto& issue : issues) {
        if (issue.getId() == id) return &issue;
    }
    return nullptr;
}

// Deterministic Agent Score Compounding Algorithm
//
// For each registered agent, compute a compound score relative to the issue:
//   score = (agent.getSuccessRate() * 10.0)     -- historic performance weight
//         + (6 - issue.getPriority())           -- priority urgency weight (1..5 → 5..1)
//
// The agent with the highest score is selected.
const Agent* IssueBoard::findBestAgent(const Issue& issue) const {
    if (agentRegistry.empty()) return nullptr;

    const Agent* bestAgent = nullptr;
    double bestScore = -1.0;

    for (const auto& [name, agent] : agentRegistry) {
        double score = agent.computeScore(issue);
        if (score > bestScore) {
            bestScore = score;
            bestAgent = &agent;
        }
    }
    return bestAgent;
}

void IssueBoard::emitStateChange(int issueId,
                                  const std::string& newStatus,
                                  const std::string& agent) const {
    std::cout << "{\"event\":\"state_change\",\"issue\":" << issueId
              << ",\"status\":\"" << newStatus << "\"";
    if (!agent.empty()) {
        std::cout << ",\"agent\":\"" << agent << "\"";
    }
    std::cout << "}\n";
}

// ── Agent Registry ─────────────────────────────────────────────────────────

void IssueBoard::registerAgent(Agent agent) {
    if (agent.getName().empty()) return;
    agentRegistry[agent.getName()] = std::move(agent);
}

// ── Issue Management ───────────────────────────────────────────────────────

void IssueBoard::addIssue(const std::string& title,
                           const std::string& desc,
                           const std::string& tag,
                           int priority) {
    // Validate non-empty required fields
    if (title.empty() || desc.empty() || tag.empty()) {
        throw std::invalid_argument("Issue fields (title, description, tag) cannot be empty.");
    }
    // Validate priority range [1, 5]
    if (priority < 1 || priority > 5) {
        throw std::invalid_argument(
            "Priority must be between 1 (CRITICAL) and 5 (MINIMAL). Got: " +
            std::to_string(priority));
    }

    int nextId = static_cast<int>(issues.size()) + 1;
    issues.emplace_back(nextId, title, desc, tag, priority);

    std::cout << "{\"status\":\"success\","
              << "\"message\":\"Issue created\","
              << "\"id\":" << nextId << ","
              << "\"title\":\"" << title << "\","
              << "\"tag\":\"" << tag << "\","
              << "\"priority\":\"" << issues.back().getPriorityLabel() << "\"}\n";
}

void IssueBoard::assignIssue(int id, const std::string& agentName) {
    Issue* issue = findIssueById(id);
    if (!issue) {
        throw std::invalid_argument("Issue ID not found: " + std::to_string(id));
    }

    auto it = agentRegistry.find(agentName);
    if (it == agentRegistry.end()) {
        throw std::invalid_argument(
            "Agent not registered: \"" + agentName + "\". "
            "Use --list-agents to see available agents.");
    }

    // Manual assignment overrides any existing assignee
    issue->setAssignee(agentName);
    issue->setStatus(IssueStatus::CLAIMED);
    emitStateChange(id, "CLAIMED", agentName);
}

void IssueBoard::updateIssueStatus(int id, const std::string& statusStr) {
    Issue* issue = findIssueById(id);
    if (!issue) {
        throw std::invalid_argument("Issue ID not found: " + std::to_string(id));
    }

    // parseStatusFromString throws invalid_argument on unknown token
    IssueStatus newStatus = Issue::parseStatusFromString(statusStr);
    issue->setStatus(newStatus);
    emitStateChange(id, statusStr);
}

// ── State-Machine Lifecycle ────────────────────────────────────────────────

void IssueBoard::processNextLifecycleStep() {
    bool processedAny = false;

    for (auto& issue : issues) {

        // ── State 1: ENQUEUED → CLAIMED ──────────────────────────────────
        // Score-compounding agent selection assigns the best-fit agent.
        if (issue.getStatus() == IssueStatus::ENQUEUED) {
            const Agent* best = findBestAgent(issue);

            if (!best) {
                std::cerr << "{\"status\":\"warning\","
                          << "\"message\":\"No agents registered; cannot claim issue "
                          << issue.getId() << "\"}\n";
                continue;
            }

            issue.setAssignee(best->getName());
            issue.setStatus(IssueStatus::CLAIMED);
            emitStateChange(issue.getId(), "CLAIMED", best->getName());
            processedAny = true;
            break; // Single mutation per poll cycle

        // ── State 2: CLAIMED → RUNNING ────────────────────────────────────
        } else if (issue.getStatus() == IssueStatus::CLAIMED) {
            issue.setStatus(IssueStatus::RUNNING);
            emitStateChange(issue.getId(), "RUNNING", issue.getAssignee());
            processedAny = true;
            break;

        // ── State 3: RUNNING → COMPLETED | BLOCKED ────────────────────────
        // Invokes polymorphic executeTask() on the assigned agent and records result.
        } else if (issue.getStatus() == IssueStatus::RUNNING) {
            auto it = agentRegistry.find(issue.getAssignee());

            if (it == agentRegistry.end()) {
                // Assigned agent was de-registered after assignment — force-block
                issue.setStatus(IssueStatus::BLOCKED);
                emitStateChange(issue.getId(), "BLOCKED");
                processedAny = true;
                break;
            }

            // Direct method call on concrete Agent instance
            bool success = it->second.executeTask(issue);
            it->second.recordResult(success);  // Update historic performance stats

            if (success) {
                issue.setStatus(IssueStatus::COMPLETED);
                emitStateChange(issue.getId(), "COMPLETED", issue.getAssignee());
            } else {
                issue.setStatus(IssueStatus::BLOCKED);
                emitStateChange(issue.getId(), "BLOCKED", issue.getAssignee());
            }
            processedAny = true;
            break;
        }
    }

    if (!processedAny) {
        std::cout << "{\"status\":\"idle\","
                  << "\"message\":\"No actionable issues found. All issues are terminal "
                  << "(COMPLETED or BLOCKED) or board is empty.\"}\n";
    }
}

// ── JSON Output ────────────────────────────────────────────────────────────

void IssueBoard::printBoardJSON() const {
    std::cout << "[";
    for (size_t i = 0; i < issues.size(); ++i) {
        const auto& issue = issues[i];
        std::cout << "{"
                  << "\"id\":"          << issue.getId()
                  << ",\"title\":\""    << issue.getTitle()    << "\""
                  << ",\"tag\":\""      << issue.getTag()      << "\""
                  << ",\"priority\":\""   << issue.getPriorityLabel() << "\""
                  << ",\"priority_val\":" << issue.getPriority()
                  << ",\"status\":\""   << issue.getStatusString() << "\""
                  << ",\"assignee\":\"" << issue.getAssignee() << "\""
                  << "}";
        if (i + 1 < issues.size()) std::cout << ",";
    }
    std::cout << "]\n";
}

void IssueBoard::printAgentsJSON() const {
    std::cout << "[";
    bool first = true;
    for (const auto& [name, agent] : agentRegistry) {
        if (!first) std::cout << ",";
        first = false;

        // Format success rate as a percentage string (two decimal places)
        std::ostringstream rateStr;
        rateStr << std::fixed << std::setprecision(2)
                << (agent.getSuccessRate() * 100.0);

        std::cout << "{"
                  << "\"name\":\""       << agent.getName()         << "\""
                  << ",\"tasks_total\":"  << agent.getTotalCount()
                  << ",\"tasks_success\":" << agent.getSuccessCount()
                  << ",\"success_rate\":\"" << rateStr.str() << "%\""
                  << "}";
    }
    std::cout << "]\n";
}