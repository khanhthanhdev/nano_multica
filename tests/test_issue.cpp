#include "TestFramework.hpp"
#include "Issue.hpp"

TEST_CASE(Issue_DefaultConstruction) {
    Issue i(1, "T", "D", "auth", PRIORITY_MEDIUM);
    ASSERT_EQ(i.getId(),          1);
    ASSERT_EQ(i.getTitle(),       std::string("T"));
    ASSERT_EQ(i.getDescription(), std::string("D"));
    ASSERT_EQ(i.getTag(),         std::string("auth"));
    ASSERT_EQ(i.getStatus(),      IssueStatus::ENQUEUED);
    ASSERT_EQ(i.getAssignee(),    std::string("none"));
    ASSERT_EQ(i.getPriority(),    PRIORITY_MEDIUM);
}

TEST_CASE(Issue_DefaultPriorityMedium) {
    Issue i(7, "T", "D", "auth");
    ASSERT_EQ(i.getPriority(), PRIORITY_MEDIUM);
}

TEST_CASE(Issue_StatusStringMapping) {
    Issue i(1, "T", "D", "auth");
    ASSERT_EQ(i.getStatusString(), std::string("ENQUEUED"));
    i.setStatus(IssueStatus::CLAIMED);
    ASSERT_EQ(i.getStatusString(), std::string("CLAIMED"));
    i.setStatus(IssueStatus::RUNNING);
    ASSERT_EQ(i.getStatusString(), std::string("RUNNING"));
    i.setStatus(IssueStatus::COMPLETED);
    ASSERT_EQ(i.getStatusString(), std::string("COMPLETED"));
    i.setStatus(IssueStatus::BLOCKED);
    ASSERT_EQ(i.getStatusString(), std::string("BLOCKED"));
}

TEST_CASE(Issue_PriorityLabelMapping) {
    ASSERT_EQ(Issue(1,"x","y","t",1).getPriorityLabel(), std::string("CRITICAL"));
    ASSERT_EQ(Issue(1,"x","y","t",2).getPriorityLabel(), std::string("HIGH"));
    ASSERT_EQ(Issue(1,"x","y","t",3).getPriorityLabel(), std::string("MEDIUM"));
    ASSERT_EQ(Issue(1,"x","y","t",4).getPriorityLabel(), std::string("LOW"));
    ASSERT_EQ(Issue(1,"x","y","t",5).getPriorityLabel(), std::string("MINIMAL"));
}

TEST_CASE(Issue_ParseStatusFromInt_Valid) {
    ASSERT_EQ(Issue::parseStatus(0), IssueStatus::ENQUEUED);
    ASSERT_EQ(Issue::parseStatus(1), IssueStatus::CLAIMED);
    ASSERT_EQ(Issue::parseStatus(2), IssueStatus::RUNNING);
    ASSERT_EQ(Issue::parseStatus(3), IssueStatus::COMPLETED);
    ASSERT_EQ(Issue::parseStatus(4), IssueStatus::BLOCKED);
}

TEST_CASE(Issue_ParseStatusFromInt_InvalidDefaultsToEnqueued) {
    ASSERT_EQ(Issue::parseStatus(99),  IssueStatus::ENQUEUED);
    ASSERT_EQ(Issue::parseStatus(-1),  IssueStatus::ENQUEUED);
}

TEST_CASE(Issue_ParseStatusFromString_Valid) {
    ASSERT_EQ(Issue::parseStatusFromString("ENQUEUED"),  IssueStatus::ENQUEUED);
    ASSERT_EQ(Issue::parseStatusFromString("CLAIMED"),   IssueStatus::CLAIMED);
    ASSERT_EQ(Issue::parseStatusFromString("RUNNING"),   IssueStatus::RUNNING);
    ASSERT_EQ(Issue::parseStatusFromString("COMPLETED"), IssueStatus::COMPLETED);
    ASSERT_EQ(Issue::parseStatusFromString("BLOCKED"),   IssueStatus::BLOCKED);
}

TEST_CASE(Issue_ParseStatusFromString_InvalidThrows) {
    ASSERT_THROWS(Issue::parseStatusFromString("FOO"),   std::invalid_argument);
    ASSERT_THROWS(Issue::parseStatusFromString("queued"),std::invalid_argument);
    ASSERT_THROWS(Issue::parseStatusFromString(""),      std::invalid_argument);
}

TEST_CASE(Issue_SetPriority_ClampsInvalid) {
    Issue i(1, "T", "D", "auth", 3);
    i.setPriority(99);
    ASSERT_EQ(i.getPriority(), PRIORITY_MEDIUM);
    i.setPriority(0);
    ASSERT_EQ(i.getPriority(), PRIORITY_MEDIUM);
    i.setPriority(2);
    ASSERT_EQ(i.getPriority(), 2);
}

TEST_CASE(Issue_SetAssignee) {
    Issue i(1, "T", "D", "auth");
    i.setAssignee("Claude-3.5");
    ASSERT_EQ(i.getAssignee(), std::string("Claude-3.5"));
}
