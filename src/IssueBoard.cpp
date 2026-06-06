#include "IssueBoard.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// IssueBoard — Implementation
// ─────────────────────────────────────────────────────────────────────────────

IssueBoard::IssueBoard(std::shared_ptr<Workspace> ws)
    : workspace(std::move(ws)) {
    if (workspace) {
        workspace->load(issues, agentRegistry);
    }
}

IssueBoard::~IssueBoard() {
    if (workspace) {
        workspace->save(issues, agentRegistry);
    }
}

// ── Private Helpers ────────────────────────────────────────────────────────

Issue* IssueBoard::findIssueById(int id) {
    for (auto& issue : issues) {
        if (issue.getId() == id) {
            return &issue;
        }
    }
    return nullptr;
}

const Issue* IssueBoard::findIssueById(int id) const {
    for (const auto& issue : issues) {
        if (issue.getId() == id) {
            return &issue;
        }
    }
    return nullptr;
}

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
    std::cout << "{\"event\":\"state_change\","
              << "\"issue\":" << issueId
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
    if (title.empty() || desc.empty() || tag.empty()) {
        throw std::invalid_argument("Title, description, and tag cannot be empty.");
    }
    if (priority < 1 || priority > 5) {
        throw std::invalid_argument("Priority must be between 1 (Critical) and 5 (Minimal).");
    }

    int nextId = 1;
    if (!issues.empty()) {
        nextId = issues.back().getId() + 1;
    }

    Issue newIssue(nextId, title, desc, tag, priority);
    issues.push_back(std::move(newIssue));

    std::cout << "{\"status\":\"success\",\"message\":\"Issue created\","
              << "\"id\":" << nextId
              << ",\"title\":\"" << title << "\""
              << ",\"tag\":\"" << tag << "\""
              << ",\"priority\":\"" << issues.back().getPriorityLabel() << "\"}\n";
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

    issue->setAssignee(agentName);
    issue->setStatus(IssueStatus::CLAIMED);
    emitStateChange(id, "CLAIMED", agentName);
}

void IssueBoard::updateIssueStatus(int id, const std::string& statusStr) {
    Issue* issue = findIssueById(id);
    if (!issue) {
        throw std::invalid_argument("Issue ID not found: " + std::to_string(id));
    }

    IssueStatus status = Issue::parseStatusFromString(statusStr);
    issue->setStatus(status);
    emitStateChange(id, statusStr, issue->getAssignee());
}

// ── State-Machine Lifecycle ────────────────────────────────────────────────

void IssueBoard::processNextLifecycleStep() {
    bool processedAny = false;

    for (auto& issue : issues) {
        // Transition 1: ENQUEUED → CLAIMED
        if (issue.getStatus() == IssueStatus::ENQUEUED) {
            const Agent* best = findBestAgent(issue);

            if (!best) {
                std::cerr << "{\"status\":\"warning\",\"source\":\"IssueBoard::processNextLifecycleStep\","
                          << "\"message\":\"No agents registered to claim ENQUEUED issue " << issue.getId() << "\"}\n";
                continue;
            }

            issue.setAssignee(best->getName());
            issue.setStatus(IssueStatus::CLAIMED);
            emitStateChange(issue.getId(), "CLAIMED", best->getName());
            processedAny = true;
            break; // Mutate exactly one issue per invocation
        }
        // Transition 2: CLAIMED → RUNNING
        else if (issue.getStatus() == IssueStatus::CLAIMED) {
            issue.setStatus(IssueStatus::RUNNING);
            emitStateChange(issue.getId(), "RUNNING", issue.getAssignee());
            processedAny = true;
            break;
        }
        // Transition 3: RUNNING → COMPLETED or BLOCKED
        else if (issue.getStatus() == IssueStatus::RUNNING) {
            auto it = agentRegistry.find(issue.getAssignee());

            if (it == agentRegistry.end()) {
                // Assigned agent was de-registered after assignment — force-block
                issue.setStatus(IssueStatus::BLOCKED);
                emitStateChange(issue.getId(), "BLOCKED");
                processedAny = true;
                break;
            }

            // Dynamic execution simulation logic using concrete Agent
            bool success = it->second.executeTask(issue);
            it->second.recordResult(success);

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
        std::cout << "{\"status\":\"idle\",\"message\":\"No actionable issues found.\"}\n";
    }
}

// ── JSON Output ────────────────────────────────────────────────────────────

void IssueBoard::printBoardJSON() const {
    std::cout << "[";
    bool first = true;
    for (const auto& issue : issues) {
        if (!first) std::cout << ",";
        first = false;
        std::cout << "{"
                  << "\"id\":" << issue.getId()
                  << ",\"title\":\"" << issue.getTitle() << "\""
                  << ",\"tag\":\"" << issue.getTag() << "\""
                  << ",\"priority\":\"" << issue.getPriorityLabel() << "\""
                  << ",\"priority_val\":" << issue.getPriority()
                  << ",\"status\":\"" << issue.getStatusString() << "\""
                  << ",\"assignee\":\"" << issue.getAssignee() << "\""
                  << "}";
    }
    std::cout << "]\n";
}

void IssueBoard::printAgentsJSON() const {
    std::cout << "[";
    bool first = true;
    for (const auto& [name, agent] : agentRegistry) {
        if (!first) std::cout << ",";
        first = false;

        std::ostringstream rateStr;
        rateStr << std::fixed << std::setprecision(2)
                << (agent.getSuccessRate() * 100.0);

        std::cout << "{"
                  << "\"name\":\""       << agent.getName() << "\""
                  << ",\"specialty\":\"" << agent.getSpecialty() << "\""
                  << ",\"tasks_total\":"  << agent.getTotalCount()
                  << ",\"tasks_success\":" << agent.getSuccessCount()
                  << ",\"success_rate\":\"" << rateStr.str() << "%\""
                  << "}";
    }
    std::cout << "]\n";
}