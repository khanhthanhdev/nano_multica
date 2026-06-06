#include "TestFramework.hpp"
#include "MulticaWorkspace.hpp"
#include "Issue.hpp"
#include "Agent.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
const std::string kIssuesPath = "/tmp/multica_test_ws_issues.dat";
const std::string kAgentsPath = "/tmp/multica_test_ws_agents.dat";

void clean() {
    std::remove(kIssuesPath.c_str());
    std::remove(kAgentsPath.c_str());
}
} // namespace

TEST_CASE(Workspace_LoadMissingIssuesFile_NoThrow) {
    clean();
    MulticaWorkspace ws(kIssuesPath, kAgentsPath);
    std::vector<Issue> issues;
    ws.loadIssues(issues);
    ASSERT_EQ(issues.size(), static_cast<size_t>(0));
    clean();
}

TEST_CASE(Workspace_LoadMissingAgentsFile_NoThrow) {
    clean();
    MulticaWorkspace ws(kIssuesPath, kAgentsPath);
    std::unordered_map<std::string, Agent> reg;
    reg["Claude-3.5"] = Agent("Claude-3.5", "auth");
    ws.loadAgentStats(reg);
    ASSERT_EQ(reg["Claude-3.5"].getTotalCount(), 0);
    clean();
}

TEST_CASE(Workspace_SaveAndLoadIssues_RoundTrip) {
    clean();
    {
        MulticaWorkspace ws(kIssuesPath, kAgentsPath);
        std::vector<Issue> issues;
        Issue a(1, "Title One", "Desc One", "auth",     PRIORITY_CRITICAL);
        a.setAssignee("Claude-3.5");
        a.setStatus(IssueStatus::CLAIMED);
        Issue b(2, "Title Two", "Desc Two", "database", PRIORITY_LOW);
        b.setAssignee("Cursor-Composer");
        b.setStatus(IssueStatus::RUNNING);
        issues.push_back(a);
        issues.push_back(b);
        ws.saveIssues(issues);
    }
    {
        MulticaWorkspace ws(kIssuesPath, kAgentsPath);
        std::vector<Issue> loaded;
        ws.loadIssues(loaded);
        ASSERT_EQ(loaded.size(), static_cast<size_t>(2));

        ASSERT_EQ(loaded[0].getId(),          1);
        ASSERT_EQ(loaded[0].getTitle(),       std::string("Title One"));
        ASSERT_EQ(loaded[0].getDescription(), std::string("Desc One"));
        ASSERT_EQ(loaded[0].getTag(),         std::string("auth"));
        ASSERT_EQ(loaded[0].getStatus(),      IssueStatus::CLAIMED);
        ASSERT_EQ(loaded[0].getAssignee(),    std::string("Claude-3.5"));
        ASSERT_EQ(loaded[0].getPriority(),    PRIORITY_CRITICAL);

        ASSERT_EQ(loaded[1].getId(),          2);
        ASSERT_EQ(loaded[1].getStatus(),      IssueStatus::RUNNING);
        ASSERT_EQ(loaded[1].getAssignee(),    std::string("Cursor-Composer"));
        ASSERT_EQ(loaded[1].getPriority(),    PRIORITY_LOW);
    }
    clean();
}

TEST_CASE(Workspace_SaveAndLoadAgentStats_RoundTrip) {
    clean();
    {
        MulticaWorkspace ws(kIssuesPath, kAgentsPath);
        std::unordered_map<std::string, Agent> reg;
        Agent claude("Claude-3.5", "auth");
        claude.recordResult(true);
        claude.recordResult(true);
        claude.recordResult(false);
        reg["Claude-3.5"] = claude;
        ws.saveAgentStats(reg);
    }
    {
        MulticaWorkspace ws(kIssuesPath, kAgentsPath);
        std::unordered_map<std::string, Agent> reg;
        reg["Claude-3.5"] = Agent("Claude-3.5", "auth");
        ws.loadAgentStats(reg);
        ASSERT_EQ(reg["Claude-3.5"].getTotalCount(),   3);
        ASSERT_EQ(reg["Claude-3.5"].getSuccessCount(), 2);
    }
    clean();
}

TEST_CASE(Workspace_LoadIssues_SkipsMalformedLine) {
    clean();
    {
        std::ofstream out(kIssuesPath);
        out << "1|Good Issue|desc|auth|0|none|3\n";
        out << "this_is_garbage_no_pipes\n";
        out << "2|Another|desc|database|0|none|3\n";
    }
    MulticaWorkspace ws(kIssuesPath, kAgentsPath);
    std::vector<Issue> loaded;
    ws.loadIssues(loaded);
    ASSERT_EQ(loaded.size(), static_cast<size_t>(2));
    ASSERT_EQ(loaded[0].getId(), 1);
    ASSERT_EQ(loaded[1].getId(), 2);
    clean();
}

TEST_CASE(Workspace_LoadAgentStats_IgnoresUnknownAgent) {
    clean();
    {
        std::ofstream out(kAgentsPath);
        out << "Claude-3.5|auth|3|5\n";
        out << "Ghost-Agent|database|99|100\n";
    }
    MulticaWorkspace ws(kIssuesPath, kAgentsPath);
    std::unordered_map<std::string, Agent> reg;
    reg["Claude-3.5"] = Agent("Claude-3.5", "auth");
    ws.loadAgentStats(reg);
    ASSERT_EQ(reg["Claude-3.5"].getTotalCount(),   5);
    ASSERT_EQ(reg["Claude-3.5"].getSuccessCount(), 3);
    clean();
}

TEST_CASE(Workspace_LoadAgentStats_SkipsMalformedLine) {
    clean();
    {
        std::ofstream out(kAgentsPath);
        out << "Claude-3.5|auth|abc|5\n";
    }
    MulticaWorkspace ws(kIssuesPath, kAgentsPath);
    std::unordered_map<std::string, Agent> reg;
    reg["Claude-3.5"] = Agent("Claude-3.5", "auth");
    ws.loadAgentStats(reg);
    ASSERT_EQ(reg["Claude-3.5"].getTotalCount(), 0);
    clean();
}

TEST_CASE(Workspace_SaveIssues_OverwritesPreviousContent) {
    clean();
    MulticaWorkspace ws(kIssuesPath, kAgentsPath);
    std::vector<Issue> first;
    first.push_back(Issue(1, "First", "d", "auth", 3));
    ws.saveIssues(first);

    std::vector<Issue> second;
    second.push_back(Issue(1, "Second", "d", "auth", 3));
    ws.saveIssues(second);

    std::vector<Issue> loaded;
    ws.loadIssues(loaded);
    ASSERT_EQ(loaded.size(), static_cast<size_t>(1));
    ASSERT_EQ(loaded[0].getTitle(), std::string("Second"));
    clean();
}
