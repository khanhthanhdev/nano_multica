#include "TestFramework.hpp"
#include "IssueBoard.hpp"
#include "Agent.hpp"
#include "MulticaWorkspace.hpp"
#include <cstdio>
#include <memory>
#include <string>

namespace {

std::shared_ptr<MulticaWorkspace> makeTempWorkspace(const std::string& tag) {
    const std::string iPath = "/tmp/multica_test_" + tag + "_issues.dat";
    const std::string aPath = "/tmp/multica_test_" + tag + "_agents.dat";
    std::remove(iPath.c_str());
    std::remove(aPath.c_str());
    return std::make_shared<MulticaWorkspace>(iPath, aPath);
}

} // namespace

TEST_CASE(IssueBoard_AddIssue_AutoIncrementsId) {
    auto ws = makeTempWorkspace("addinc");
    IssueBoard board(ws);
    board.addIssue("A", "desc", "auth",     PRIORITY_MEDIUM);
    board.addIssue("B", "desc", "database", PRIORITY_MEDIUM);
    board.addIssue("C", "desc", "frontend", PRIORITY_MEDIUM);
    const auto& list = board.getIssues();
    ASSERT_EQ(list.size(), static_cast<size_t>(3));
    ASSERT_EQ(list[0].getId(), 1);
    ASSERT_EQ(list[1].getId(), 2);
    ASSERT_EQ(list[2].getId(), 3);
}

TEST_CASE(IssueBoard_AddIssue_RejectsEmptyFields) {
    auto ws = makeTempWorkspace("empty");
    IssueBoard board(ws);
    ASSERT_THROWS(board.addIssue("",  "d", "auth"), std::invalid_argument);
    ASSERT_THROWS(board.addIssue("t", "",  "auth"), std::invalid_argument);
    ASSERT_THROWS(board.addIssue("t", "d", ""),     std::invalid_argument);
}

TEST_CASE(IssueBoard_AddIssue_RejectsOutOfRangePriority) {
    auto ws = makeTempWorkspace("prio");
    IssueBoard board(ws);
    ASSERT_THROWS(board.addIssue("t", "d", "auth", 0),  std::invalid_argument);
    ASSERT_THROWS(board.addIssue("t", "d", "auth", 6),  std::invalid_argument);
    ASSERT_THROWS(board.addIssue("t", "d", "auth", -1), std::invalid_argument);
}

TEST_CASE(IssueBoard_AddIssue_AcceptsBoundaryPriorities) {
    auto ws = makeTempWorkspace("priook");
    IssueBoard board(ws);
    board.addIssue("t", "d", "auth", 1);
    board.addIssue("t", "d", "auth", 5);
    ASSERT_EQ(board.getIssues().size(), static_cast<size_t>(2));
}

TEST_CASE(IssueBoard_RegisterAgent_OverwritesOnDuplicate) {
    auto ws = makeTempWorkspace("dup");
    IssueBoard board(ws);
    board.registerAgent(Agent("Claude-3.5", "auth"));
    board.registerAgent(Agent("Claude-3.5", "frontend"));
    const auto& agents = board.getAgents();
    ASSERT_EQ(agents.size(), static_cast<size_t>(1));
    ASSERT_EQ(agents.at("Claude-3.5").getSpecialty(), std::string("frontend"));
}

TEST_CASE(IssueBoard_RegisterAgent_RejectsEmptyName) {
    auto ws = makeTempWorkspace("regempty");
    IssueBoard board(ws);
    board.registerAgent(Agent("", "auth"));
    ASSERT_EQ(board.getAgents().size(), static_cast<size_t>(0));
}

TEST_CASE(IssueBoard_AssignIssue_NotFoundThrows) {
    auto ws = makeTempWorkspace("nf");
    IssueBoard board(ws);
    board.registerAgent(Agent("Claude-3.5", "auth"));
    ASSERT_THROWS(board.assignIssue(42, "Claude-3.5"), std::invalid_argument);
}

TEST_CASE(IssueBoard_AssignIssue_UnknownAgentThrows) {
    auto ws = makeTempWorkspace("ua");
    IssueBoard board(ws);
    board.addIssue("T", "D", "auth", PRIORITY_MEDIUM);
    ASSERT_THROWS(board.assignIssue(1, "No-Such-Agent"), std::invalid_argument);
}

TEST_CASE(IssueBoard_AssignIssue_SetsClaimed) {
    auto ws = makeTempWorkspace("claimed");
    IssueBoard board(ws);
    board.registerAgent(Agent("Claude-3.5", "auth"));
    board.addIssue("T", "D", "auth", PRIORITY_MEDIUM);
    board.assignIssue(1, "Claude-3.5");
    const auto& list = board.getIssues();
    ASSERT_EQ(list[0].getStatus(),   IssueStatus::CLAIMED);
    ASSERT_EQ(list[0].getAssignee(), std::string("Claude-3.5"));
}

TEST_CASE(IssueBoard_UpdateIssueStatus_Valid) {
    auto ws = makeTempWorkspace("upd");
    IssueBoard board(ws);
    board.addIssue("T", "D", "auth", PRIORITY_MEDIUM);
    board.updateIssueStatus(1, "BLOCKED");
    ASSERT_EQ(board.getIssues()[0].getStatus(), IssueStatus::BLOCKED);
}

TEST_CASE(IssueBoard_UpdateIssueStatus_InvalidThrows) {
    auto ws = makeTempWorkspace("updbad");
    IssueBoard board(ws);
    board.addIssue("T", "D", "auth", PRIORITY_MEDIUM);
    ASSERT_THROWS(board.updateIssueStatus(1, "FOO"), std::invalid_argument);
}

TEST_CASE(IssueBoard_UpdateIssueStatus_NotFoundThrows) {
    auto ws = makeTempWorkspace("updnf");
    IssueBoard board(ws);
    ASSERT_THROWS(board.updateIssueStatus(99, "BLOCKED"), std::invalid_argument);
}

TEST_CASE(IssueBoard_Lifecycle_EnqueuedToClaimedPicksSpecialist) {
    auto ws = makeTempWorkspace("lclaim");
    IssueBoard board(ws);
    board.registerAgent(Agent("Claude-3.5",     "auth"));
    board.registerAgent(Agent("Cursor-Composer", "database"));
    board.registerAgent(Agent("Gemini-Advanced", "frontend"));
    board.addIssue("Auth bug", "OAuth broken", "auth", PRIORITY_MEDIUM);
    bool progressed = board.processNextLifecycleStep();
    ASSERT_TRUE(progressed);
    const auto& list = board.getIssues();
    ASSERT_EQ(list[0].getStatus(),   IssueStatus::CLAIMED);
    ASSERT_EQ(list[0].getAssignee(), std::string("Claude-3.5"));
}

TEST_CASE(IssueBoard_Lifecycle_ClaimedToRunning) {
    auto ws = makeTempWorkspace("lrun");
    IssueBoard board(ws);
    board.registerAgent(Agent("Claude-3.5", "auth"));
    board.addIssue("T", "D", "auth", PRIORITY_MEDIUM);
    board.processNextLifecycleStep();
    bool progressed = board.processNextLifecycleStep();
    ASSERT_TRUE(progressed);
    ASSERT_EQ(board.getIssues()[0].getStatus(), IssueStatus::RUNNING);
}

TEST_CASE(IssueBoard_Lifecycle_RunningToCompletedOnSuccess) {
    auto ws = makeTempWorkspace("ldone");
    IssueBoard board(ws);
    board.registerAgent(Agent("Cursor-Composer", "database"));
    board.addIssue("Optimize Index", "speed up", "database", PRIORITY_MEDIUM);
    board.processNextLifecycleStep();
    board.processNextLifecycleStep();
    bool progressed = board.processNextLifecycleStep();
    ASSERT_TRUE(progressed);
    ASSERT_EQ(board.getIssues()[0].getStatus(), IssueStatus::COMPLETED);
    ASSERT_EQ(board.getAgents().at("Cursor-Composer").getTotalCount(), 1);
    ASSERT_EQ(board.getAgents().at("Cursor-Composer").getSuccessCount(), 1);
}

TEST_CASE(IssueBoard_Lifecycle_RunningToBlockedOnFailure) {
    auto ws = makeTempWorkspace("lblock");
    IssueBoard board(ws);
    board.registerAgent(Agent("Claude-3.5", "auth"));
    board.addIssue("Fix Server", "severe CRASH on boot", "auth", PRIORITY_MEDIUM);
    board.processNextLifecycleStep();
    board.processNextLifecycleStep();
    bool progressed = board.processNextLifecycleStep();
    ASSERT_TRUE(progressed);
    ASSERT_EQ(board.getIssues()[0].getStatus(), IssueStatus::BLOCKED);
    ASSERT_EQ(board.getAgents().at("Claude-3.5").getTotalCount(),   1);
    ASSERT_EQ(board.getAgents().at("Claude-3.5").getSuccessCount(), 0);
}

TEST_CASE(IssueBoard_Lifecycle_NoActionableReturnsFalse) {
    auto ws = makeTempWorkspace("lidle");
    IssueBoard board(ws);
    board.registerAgent(Agent("Claude-3.5", "auth"));
    board.addIssue("T", "D", "auth", PRIORITY_MEDIUM);
    board.processNextLifecycleStep();
    board.processNextLifecycleStep();
    board.processNextLifecycleStep();
    bool progressed = board.processNextLifecycleStep();
    ASSERT_FALSE(progressed);
}

TEST_CASE(IssueBoard_Lifecycle_NoAgentsEnqueuedStaysIdle) {
    auto ws = makeTempWorkspace("noagents");
    IssueBoard board(ws);
    board.addIssue("T", "D", "auth", PRIORITY_MEDIUM);
    bool progressed = board.processNextLifecycleStep();
    ASSERT_FALSE(progressed);
    ASSERT_EQ(board.getIssues()[0].getStatus(), IssueStatus::ENQUEUED);
}

TEST_CASE(IssueBoard_Lifecycle_FullCycleMultiIssueSequential) {
    auto ws = makeTempWorkspace("lmulti");
    IssueBoard board(ws);
    board.registerAgent(Agent("Cursor-Composer", "database"));
    board.addIssue("A", "a", "database", PRIORITY_MEDIUM);
    board.addIssue("B", "b", "database", PRIORITY_MEDIUM);
    for (int i = 0; i < 6; ++i) board.processNextLifecycleStep();
    const auto& list = board.getIssues();
    ASSERT_EQ(list[0].getStatus(), IssueStatus::COMPLETED);
    ASSERT_EQ(list[1].getStatus(), IssueStatus::COMPLETED);
}

TEST_CASE(IssueBoard_Lifecycle_DeRegisteredAgentForcesBlock) {
    auto ws = makeTempWorkspace("dereg");
    {
        IssueBoard board(ws);
        board.registerAgent(Agent("Claude-3.5", "auth"));
        board.addIssue("T", "D", "auth", PRIORITY_MEDIUM);
        board.processNextLifecycleStep();
        board.processNextLifecycleStep();
    }
    auto ws2 = std::make_shared<MulticaWorkspace>(
        "/tmp/multica_test_dereg_issues.dat",
        "/tmp/multica_test_dereg_agents.dat");
    IssueBoard board2(ws2);
    // No agents registered this time — re-registered one is gone.
    bool progressed = board2.processNextLifecycleStep();
    ASSERT_TRUE(progressed);
    ASSERT_EQ(board2.getIssues()[0].getStatus(), IssueStatus::BLOCKED);
}

TEST_CASE(IssueBoard_Persistence_RoundTripRestoresIssues) {
    const std::string iPath = "/tmp/multica_test_persist_issues.dat";
    const std::string aPath = "/tmp/multica_test_persist_agents.dat";
    std::remove(iPath.c_str());
    std::remove(aPath.c_str());

    {
        auto ws = std::make_shared<MulticaWorkspace>(iPath, aPath);
        IssueBoard board(ws);
        board.addIssue("Persisted", "after restart", "auth", PRIORITY_HIGH);
    }

    auto ws2 = std::make_shared<MulticaWorkspace>(iPath, aPath);
    IssueBoard board2(ws2);
    const auto& list = board2.getIssues();
    ASSERT_EQ(list.size(), static_cast<size_t>(1));
    ASSERT_EQ(list[0].getTitle(),       std::string("Persisted"));
    ASSERT_EQ(list[0].getTag(),         std::string("auth"));
    ASSERT_EQ(list[0].getPriority(),    PRIORITY_HIGH);
    ASSERT_EQ(list[0].getPriorityLabel(), std::string("HIGH"));

    std::remove(iPath.c_str());
    std::remove(aPath.c_str());
}
