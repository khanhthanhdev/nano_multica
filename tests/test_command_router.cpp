#include "TestFramework.hpp"
#include "CommandRouter.hpp"
#include "IssueBoard.hpp"
#include "MulticaWorkspace.hpp"
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace {

std::shared_ptr<MulticaWorkspace> makeTempWorkspace(const std::string& tag) {
    const std::string iPath = "/tmp/multica_test_router_" + tag + "_issues.dat";
    const std::string aPath = "/tmp/multica_test_router_" + tag + "_agents.dat";
    std::remove(iPath.c_str());
    std::remove(aPath.c_str());
    return std::make_shared<MulticaWorkspace>(iPath, aPath);
}

} // namespace

TEST_CASE(Router_HelpFlag_DoesNotThrow) {
    auto ws = makeTempWorkspace("help");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--help"};
    CommandRouter router(2, const_cast<char**>(argv));
    router.route(board);
    ASSERT_TRUE(true);
}

TEST_CASE(Router_NoFlag_Throws) {
    auto ws = makeTempWorkspace("noflag");
    IssueBoard board(ws);
    const char* argv[] = {"multica"};
    CommandRouter router(1, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}

TEST_CASE(Router_UnknownFlag_Throws) {
    auto ws = makeTempWorkspace("unk");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--unknown-flag"};
    CommandRouter router(2, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}

TEST_CASE(Router_CreateIssue_MissingArgs_Throws) {
    auto ws = makeTempWorkspace("cmiss");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--create-issue", "Title"};
    CommandRouter router(3, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}

TEST_CASE(Router_CreateIssue_InvalidPriority_Throws) {
    auto ws = makeTempWorkspace("cprio");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--create-issue", "T", "D", "auth", "9"};
    CommandRouter router(6, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}

TEST_CASE(Router_CreateIssue_NonNumericPriority_Throws) {
    auto ws = makeTempWorkspace("cnp");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--create-issue", "T", "D", "auth", "high"};
    CommandRouter router(6, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}

TEST_CASE(Router_CreateIssue_PartialNumeric_Throws) {
    auto ws = makeTempWorkspace("cpart");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--create-issue", "T", "D", "auth", "2abc"};
    CommandRouter router(6, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}

TEST_CASE(Router_CreateIssue_ValidAddsIssue) {
    auto ws = makeTempWorkspace("cok");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--create-issue", "T", "D", "auth", "2"};
    CommandRouter router(6, const_cast<char**>(argv));
    router.route(board);
    const auto& list = board.getIssues();
    ASSERT_EQ(list.size(), static_cast<size_t>(1));
    ASSERT_EQ(list[0].getTitle(),          std::string("T"));
    ASSERT_EQ(list[0].getPriorityLabel(),  std::string("HIGH"));
}

TEST_CASE(Router_AssignIssue_NonNumericId_Throws) {
    auto ws = makeTempWorkspace("aid");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--assign-issue", "abc", "Claude"};
    CommandRouter router(4, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}

TEST_CASE(Router_UpdateStatus_InvalidString_Throws) {
    auto ws = makeTempWorkspace("us");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--update-status", "1", "FOO"};
    CommandRouter router(4, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}

TEST_CASE(Router_UpdateStatus_NumericId_Throws) {
    auto ws = makeTempWorkspace("usnum");
    IssueBoard board(ws);
    const char* argv[] = {"multica", "--update-status", "x", "BLOCKED"};
    CommandRouter router(4, const_cast<char**>(argv));
    ASSERT_THROWS(router.route(board), std::invalid_argument);
}
