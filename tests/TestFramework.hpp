#pragma once
#include <cstdio>
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Provide stream operators for domain enums so ASSERT_EQ can print them on failure.
#include "Issue.hpp"

inline std::ostream& operator<<(std::ostream& os, IssueStatus s) {
    switch (s) {
        case IssueStatus::ENQUEUED:  return os << "ENQUEUED";
        case IssueStatus::CLAIMED:   return os << "CLAIMED";
        case IssueStatus::RUNNING:   return os << "RUNNING";
        case IssueStatus::COMPLETED: return os << "COMPLETED";
        case IssueStatus::BLOCKED:   return os << "BLOCKED";
    }
    return os << "UNKNOWN";
}

namespace testing {

struct TestCase {
    std::string        name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

struct Registrar {
    inline Registrar(const std::string& name, std::function<void()> fn) {
        registry().push_back({name, std::move(fn)});
    }
};

inline int&         failCount()    { static int n = 0; return n; }
inline int&         passCount()    { static int n = 0; return n; }
inline std::string& currentName()  { static std::string s; return s; }

inline void recordPass() { passCount()++; }

inline void recordFail(const std::string& msg) {
    std::cerr << "\n  [FAIL] " << currentName() << "\n"
              << "    " << msg << "\n";
    failCount()++;
}

} // namespace testing

#define TEST_CASE(test_name)                                                  \
    static void test_body_##test_name();                                      \
    inline testing::Registrar reg_##test_name(                                \
        #test_name, test_body_##test_name);                                   \
    static void test_body_##test_name()

#define ASSERT_TRUE(cond)                                                     \
    do {                                                                      \
        if (!(cond)) {                                                        \
            std::ostringstream _oss;                                          \
            _oss << __FILE__ << ":" << __LINE__ << "\n    "                   \
                 << "expected `" << #cond << "` to be true";                  \
            testing::recordFail(_oss.str());                                  \
            return;                                                           \
        }                                                                     \
        testing::recordPass();                                                \
    } while (0)

#define ASSERT_FALSE(cond)                                                    \
    do {                                                                      \
        if ((cond)) {                                                         \
            std::ostringstream _oss;                                          \
            _oss << __FILE__ << ":" << __LINE__ << "\n    "                   \
                 << "expected `" << #cond << "` to be false";                 \
            testing::recordFail(_oss.str());                                  \
            return;                                                           \
        }                                                                     \
        testing::recordPass();                                                \
    } while (0)

#define ASSERT_EQ(actual, expected)                                           \
    do {                                                                      \
        auto _a = (actual);                                                   \
        auto _b = (expected);                                                 \
        if (!(_a == _b)) {                                                    \
            std::ostringstream _oss;                                          \
            _oss << __FILE__ << ":" << __LINE__ << "\n    "                   \
                 << "expected `" << #actual << "` == `" << #expected << "`\n"\
                 << "    actual   = " << _a  << "\n"                          \
                 << "    expected = " << _b;                                  \
            testing::recordFail(_oss.str());                                  \
            return;                                                           \
        }                                                                     \
        testing::recordPass();                                                \
    } while (0)

#define ASSERT_THROWS(stmt, ExType)                                           \
    do {                                                                      \
        bool _threw = false;                                                  \
        try { (void)(stmt); }                                                 \
        catch (const ExType&) { _threw = true; }                              \
        catch (...) {}                                                        \
        if (!_threw) {                                                        \
            std::ostringstream _oss;                                          \
            _oss << __FILE__ << ":" << __LINE__ << "\n    "                   \
                 << "expected `" << #stmt << "` to throw " << #ExType;        \
            testing::recordFail(_oss.str());                                  \
            return;                                                           \
        }                                                                     \
        testing::recordPass();                                                \
    } while (0)

inline int runAllTests() {
    const int total = static_cast<int>(testing::registry().size());
    int failedTests = 0;

    for (auto& tc : testing::registry()) {
        testing::currentName() = tc.name;
        int failsBefore = testing::failCount();
        std::cout << "[ RUN  ] " << tc.name << "\n";
        try {
            tc.fn();
        } catch (const std::exception& e) {
            testing::recordFail(std::string("uncaught std::exception: ") + e.what());
        } catch (...) {
            testing::recordFail("uncaught non-std exception");
        }
        if (testing::failCount() == failsBefore) {
            std::cout << "[  OK  ] " << tc.name << "\n";
        } else {
            ++failedTests;
        }
    }

    std::cout << "\n────────────────────────────────────────\n";
    std::cout << testing::passCount() << " assertions passed\n";
    std::cout << testing::failCount() << " assertions failed\n";
    std::cout << (total - failedTests) << "/" << total
              << " test cases passed\n";
    return failedTests == 0 ? 0 : 1;
}
