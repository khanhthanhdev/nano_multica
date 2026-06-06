#ifndef MULTICA_WORKSPACE_HPP
#define MULTICA_WORKSPACE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "Issue.hpp"
#include "Agent.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// MulticaWorkspace — Persistence Abstraction Class
//
// Encapsulates all disk I/O for the MicroMultica engine. Provides clean
// load/save interfaces for both the issue collection and agent performance
// registry, keeping IssueBoard free from raw file handling logic.
//
// Storage layout:
//   • multica_issues.dat  — Pipe-delimited CSV, one issue per line
//     Format: id|title|description|tag|status_code|assignee|priority
//
//   • multica_agents.dat  — Pipe-delimited CSV, one agent record per line
//     Format: name|success_count|total_count
// ─────────────────────────────────────────────────────────────────────────────
class MulticaWorkspace {
private:
    std::string issuesFile;  // Path to the issues persistence file
    std::string agentsFile;  // Path to the agents performance file

public:
    // Construct workspace with explicit file path targets
    MulticaWorkspace(std::string issues_path = "multica_issues.dat",
                     std::string agents_path = "multica_agents.dat");

    // ── Load Operations ────────────────────────────────────────────────────

    // Deserialize issue records from disk into the provided vector.
    // Silently returns if the file does not yet exist (first-run scenario).
    void loadIssues(std::vector<Issue>& issues) const;

    // Deserialize agent performance stats from disk and update the matching
    // agents already registered in the agentRegistry map.
    // Only updates agents whose names exist in the map (does not create new agents).
    void loadAgentStats(std::unordered_map<std::string, Agent>& registry) const;

    // ── Save Operations ────────────────────────────────────────────────────

    // Serialize all issues to disk (truncates existing file).
    // Writes to std::cerr on failure without throwing.
    void saveIssues(const std::vector<Issue>& issues) const;

    // Serialize agent performance stats to disk (truncates existing file).
    void saveAgentStats(const std::unordered_map<std::string, Agent>& registry) const;

    // ── Convenience ───────────────────────────────────────────────────────

    // Perform a full load of both issues and agent stats in one call.
    void load(std::vector<Issue>& issues,
              std::unordered_map<std::string, Agent>& registry) const;

    // Perform a full save of both issues and agent stats in one call.
    void save(const std::vector<Issue>& issues,
              const std::unordered_map<std::string, Agent>& registry) const;

    // Return the configured file paths (for diagnostics/logging)
    const std::string& getIssuesPath() const { return issuesFile; }
    const std::string& getAgentsPath() const { return agentsFile; }
};

#endif // MULTICA_WORKSPACE_HPP
