#include "MulticaWorkspace.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// MulticaWorkspace — Implementation
// ─────────────────────────────────────────────────────────────────────────────

MulticaWorkspace::MulticaWorkspace(std::string issues_path, std::string agents_path)
    : issuesFile(std::move(issues_path)), agentsFile(std::move(agents_path)) {}

// ── Load Operations ────────────────────────────────────────────────────────

void MulticaWorkspace::loadIssues(std::vector<Issue>& issues) const {
    std::ifstream file(issuesFile);
    if (!file.is_open()) {
        // Acceptable on first run — workspace initializes as empty
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string idStr, title, desc, tag, statusCodeStr, assignee, priorityStr;

        // Deserialize pipe-delimited record:
        // id | title | description | tag | status_code | assignee | priority
        std::getline(ss, idStr,       '|');
        std::getline(ss, title,       '|');
        std::getline(ss, desc,        '|');
        std::getline(ss, tag,         '|');
        std::getline(ss, statusCodeStr,'|');
        std::getline(ss, assignee,    '|');
        std::getline(ss, priorityStr, '|');

        try {
            int id         = std::stoi(idStr);
            int statusCode = std::stoi(statusCodeStr);
            int priority   = priorityStr.empty() ? PRIORITY_MEDIUM : std::stoi(priorityStr);

            Issue issue(id, title, desc, tag, priority);
            issue.setAssignee(assignee);
            issue.setStatus(Issue::parseStatus(statusCode));
            issues.push_back(std::move(issue));
        } catch (const std::exception& e) {
            std::cerr << "{\"status\":\"warning\",\"source\":\"MulticaWorkspace::loadIssues\","
                      << "\"message\":\"Skipping malformed record: " << e.what() << "\"}\n";
        }
    }
}

void MulticaWorkspace::loadAgentStats(
    std::unordered_map<std::string, std::shared_ptr<Agent>>& registry) const {

    std::ifstream file(agentsFile);
    if (!file.is_open()) return; // Not yet created — fresh workspace

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string name, specialty, successStr, totalStr;

        // Deserialize: name | specialty | success_count | total_count
        std::getline(ss, name,      '|');
        std::getline(ss, specialty, '|');
        std::getline(ss, successStr,'|');
        std::getline(ss, totalStr,  '|');

        try {
            auto it = registry.find(name);
            if (it != registry.end() && it->second) {
                // Replay historic performance data into the live agent object
                int successes = successStr.empty() ? 0 : std::stoi(successStr);
                int total     = totalStr.empty()   ? 0 : std::stoi(totalStr);
                // Record individually to keep counter semantics correct
                for (int i = 0; i < total; ++i) {
                    it->second->recordResult(i < successes);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "{\"status\":\"warning\",\"source\":\"MulticaWorkspace::loadAgentStats\","
                      << "\"message\":\"Skipping malformed agent record: " << e.what() << "\"}\n";
        }
    }
}

// ── Save Operations ────────────────────────────────────────────────────────

void MulticaWorkspace::saveIssues(const std::vector<Issue>& issues) const {
    std::ofstream file(issuesFile, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "{\"status\":\"error\",\"source\":\"MulticaWorkspace::saveIssues\","
                  << "\"message\":\"Failed to open issues file for writing: " << issuesFile << "\"}\n";
        return;
    }

    for (const auto& issue : issues) {
        // Serialize: id | title | description | tag | status_code | assignee | priority
        file << issue.getId()          << '|'
             << issue.getTitle()       << '|'
             << issue.getDescription() << '|'
             << issue.getTag()         << '|'
             << static_cast<int>(issue.getStatus()) << '|'
             << issue.getAssignee()    << '|'
             << issue.getPriority()    << '\n';
    }
}

void MulticaWorkspace::saveAgentStats(
    const std::unordered_map<std::string, std::shared_ptr<Agent>>& registry) const {

    std::ofstream file(agentsFile, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "{\"status\":\"error\",\"source\":\"MulticaWorkspace::saveAgentStats\","
                  << "\"message\":\"Failed to open agents file for writing: " << agentsFile << "\"}\n";
        return;
    }

    for (const auto& [name, agentPtr] : registry) {
        if (!agentPtr) continue;
        // Serialize: name | specialty | success_count | total_count
        file << agentPtr->getName()         << '|'
             << agentPtr->getSpecialty()    << '|'
             << agentPtr->getSuccessCount() << '|'
             << agentPtr->getTotalCount()   << '\n';
    }
}

// ── Convenience Wrappers ───────────────────────────────────────────────────

void MulticaWorkspace::load(
    std::vector<Issue>& issues,
    std::unordered_map<std::string, std::shared_ptr<Agent>>& registry) const {
    loadIssues(issues);
    loadAgentStats(registry);
}

void MulticaWorkspace::save(
    const std::vector<Issue>& issues,
    const std::unordered_map<std::string, std::shared_ptr<Agent>>& registry) const {
    saveIssues(issues);
    saveAgentStats(registry);
}
