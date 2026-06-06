#ifndef MULTICA_WORKSPACE_HPP
#define MULTICA_WORKSPACE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "Issue.hpp"
#include "Agent.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Workspace — Abstract Base Class (Interface for Dependency Injection)
//
// Defines the persistence layer contract. This interface is injected into the
// IssueBoard orchestrator.
// ─────────────────────────────────────────────────────────────────────────────
class Workspace {
public:
    virtual ~Workspace() = default;

    virtual void loadIssues(std::vector<Issue>& issues) const = 0;
    virtual void loadAgentStats(std::unordered_map<std::string, Agent>& registry) const = 0;
    virtual void saveIssues(const std::vector<Issue>& issues) const = 0;
    virtual void saveAgentStats(const std::unordered_map<std::string, Agent>& registry) const = 0;

    virtual void load(std::vector<Issue>& issues,
                      std::unordered_map<std::string, Agent>& registry) const = 0;
    virtual void save(const std::vector<Issue>& issues,
                      const std::unordered_map<std::string, Agent>& registry) const = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// MulticaWorkspace — Concrete Implementation (File-based Persistence)
// ─────────────────────────────────────────────────────────────────────────────
class MulticaWorkspace : public Workspace {
private:
    std::string issuesFile;  // Path to the issues persistence file
    std::string agentsFile;  // Path to the agents performance file

public:
    MulticaWorkspace(std::string issues_path = "multica_issues.dat",
                     std::string agents_path = "multica_agents.dat");

    void loadIssues(std::vector<Issue>& issues) const override;
    void loadAgentStats(std::unordered_map<std::string, Agent>& registry) const override;
    void saveIssues(const std::vector<Issue>& issues) const override;
    void saveAgentStats(const std::unordered_map<std::string, Agent>& registry) const override;

    void load(std::vector<Issue>& issues,
              std::unordered_map<std::string, Agent>& registry) const override;
    void save(const std::vector<Issue>& issues,
              const std::unordered_map<std::string, Agent>& registry) const override;

    const std::string& getIssuesPath() const { return issuesFile; }
    const std::string& getAgentsPath() const { return agentsFile; }
};

#endif // MULTICA_WORKSPACE_HPP
