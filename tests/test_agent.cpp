#include "TestFramework.hpp"
#include "Agent.hpp"
#include "Issue.hpp"

TEST_CASE(Agent_RecordResult_IncrementsCounters) {
    Agent a("X", "auth");
    ASSERT_EQ(a.getTotalCount(),   0);
    ASSERT_EQ(a.getSuccessCount(), 0);
    a.recordResult(true);
    a.recordResult(true);
    a.recordResult(false);
    ASSERT_EQ(a.getTotalCount(),   3);
    ASSERT_EQ(a.getSuccessCount(), 2);
}

TEST_CASE(Agent_SuccessRate_NoTasksDefaultsToOne) {
    Agent a("X", "auth");
    ASSERT_EQ(a.getSuccessRate(), 1.0);
}

TEST_CASE(Agent_SuccessRate_AllSuccess) {
    Agent a("X", "auth");
    a.recordResult(true);
    a.recordResult(true);
    ASSERT_EQ(a.getSuccessRate(), 1.0);
}

TEST_CASE(Agent_SuccessRate_AllFail) {
    Agent a("X", "auth");
    a.recordResult(false);
    a.recordResult(false);
    ASSERT_EQ(a.getSuccessRate(), 0.0);
}

TEST_CASE(Agent_SuccessRate_Half) {
    Agent a("X", "auth");
    a.recordResult(true);
    a.recordResult(false);
    ASSERT_EQ(a.getSuccessRate(), 0.5);
}

TEST_CASE(Agent_ComputeScore_SpecialtyMatchGivesTenPointBonus) {
    Issue i(1, "T", "D", "auth", 3);
    Agent matched("M",  "auth");
    Agent noMatch("N",  "database");
    double diff = matched.computeScore(i) - noMatch.computeScore(i);
    ASSERT_EQ(diff, 10.0);
}

TEST_CASE(Agent_ComputeScore_HigherSuccessRateRanksHigher) {
    Issue i(1, "T", "D", "auth", 3);
    Agent good("G", "auth");
    good.recordResult(true);
    good.recordResult(true);
    Agent bad("B", "auth");
    bad.recordResult(false);
    bad.recordResult(false);
    ASSERT_TRUE(good.computeScore(i) > bad.computeScore(i));
}

TEST_CASE(Agent_ComputeScore_HigherPriorityRanksHigher) {
    Agent a("X", "auth");
    Issue critical(1, "T", "D", "auth", PRIORITY_CRITICAL);
    Issue minimal(2,  "T", "D", "auth", PRIORITY_MINIMAL);
    ASSERT_TRUE(a.computeScore(critical) > a.computeScore(minimal));
}

TEST_CASE(Agent_ComputeScore_FrontendEscalationOnHighPriority) {
    Agent fe("Fe", "frontend");
    Issue high(1, "T", "D", "frontend", PRIORITY_CRITICAL);
    Issue low (2, "T", "D", "frontend", PRIORITY_MEDIUM);
    // diff = priority weight (2) + frontend escalation (3) = 5
    double diff = fe.computeScore(high) - fe.computeScore(low);
    ASSERT_EQ(diff, 5.0);
}

TEST_CASE(Agent_ComputeScore_NoFrontendEscalationForOtherSpecialties) {
    Agent auth("A", "auth");
    Issue high(1, "T", "D", "auth", PRIORITY_CRITICAL);
    Issue low (2, "T", "D", "auth", PRIORITY_MEDIUM);
    double diff = auth.computeScore(high) - auth.computeScore(low);
    ASSERT_EQ(diff, 2.0);
}

TEST_CASE(Agent_ExecuteTask_AuthCrashFails) {
    Agent a("A", "auth");
    Issue i(1, "Server", "severe CRASH on boot", "auth");
    ASSERT_FALSE(a.executeTask(i));
}

TEST_CASE(Agent_ExecuteTask_AuthWithoutCrashSucceeds) {
    Agent a("A", "auth");
    Issue i(1, "Server", "normal failure", "auth");
    ASSERT_TRUE(a.executeTask(i));
}

TEST_CASE(Agent_ExecuteTask_DatabaseEmptyTitleFails) {
    Agent a("A", "database");
    Issue i(1, "", "some desc", "database");
    ASSERT_FALSE(a.executeTask(i));
}

TEST_CASE(Agent_ExecuteTask_DatabaseWithTitleSucceeds) {
    Agent a("A", "database");
    Issue i(1, "Optimize", "some desc", "database");
    ASSERT_TRUE(a.executeTask(i));
}

TEST_CASE(Agent_ExecuteTask_FrontendUnverifiedCriticalFails) {
    Agent a("A", "frontend");
    Issue i(1, "UI", "this feature is UNVERIFIED", "frontend", PRIORITY_CRITICAL);
    ASSERT_FALSE(a.executeTask(i));
}

TEST_CASE(Agent_ExecuteTask_FrontendMediumPrioritySucceeds) {
    Agent a("A", "frontend");
    Issue i(1, "UI", "this feature is UNVERIFIED", "frontend", PRIORITY_MEDIUM);
    ASSERT_TRUE(a.executeTask(i));
}

TEST_CASE(Agent_ExecuteTask_ResearchOversizedDescriptionFails) {
    Agent a("A", "research");
    std::string longDesc(501, 'x');
    Issue i(1, "T", longDesc, "research");
    ASSERT_FALSE(a.executeTask(i));
}

TEST_CASE(Agent_ExecuteTask_ResearchShortDescriptionSucceeds) {
    Agent a("A", "research");
    Issue i(1, "T", "small desc", "research");
    ASSERT_TRUE(a.executeTask(i));
}

TEST_CASE(Agent_ExecuteTask_UnknownSpecialtyAlwaysSucceeds) {
    Agent a("A", "unknown_specialty");
    Issue i(1, "T", "anything", "auth");
    ASSERT_TRUE(a.executeTask(i));
}
