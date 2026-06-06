#ifndef ISSUE_HPP
#define ISSUE_HPP

#include <string>
#include <stdexcept>

// Issue priority levels: 1 (Critical) to 5 (Low)
static constexpr int PRIORITY_CRITICAL = 1;
static constexpr int PRIORITY_HIGH     = 2;
static constexpr int PRIORITY_MEDIUM   = 3;
static constexpr int PRIORITY_LOW      = 4;
static constexpr int PRIORITY_MINIMAL  = 5;

enum class IssueStatus {
    ENQUEUED  = 0,
    CLAIMED   = 1,
    RUNNING   = 2,
    COMPLETED = 3,
    BLOCKED   = 4
};

class Issue {
private:
    int         id;
    std::string title;
    std::string description;
    std::string tag;        // e.g., "auth", "database", "frontend"
    IssueStatus status;
    std::string assignee;
    int         priority;   // 1 (Critical) to 5 (Minimal); default 3

public:
    // Primary constructor — priority defaults to MEDIUM (3)
    Issue(int id, std::string t, std::string d, std::string tg, int p = PRIORITY_MEDIUM)
        : id(id), title(std::move(t)), description(std::move(d)), tag(std::move(tg)),
          status(IssueStatus::ENQUEUED), assignee("none"), priority(p) {}

    // ── Accessors ──────────────────────────────────────────────────────────
    int         getId()          const { return id; }
    std::string getTitle()       const { return title; }
    std::string getDescription() const { return description; }
    std::string getTag()         const { return tag; }
    IssueStatus getStatus()      const { return status; }
    std::string getAssignee()    const { return assignee; }
    int         getPriority()    const { return priority; }

    // ── Mutators ───────────────────────────────────────────────────────────
    void setStatus(IssueStatus s)            { status   = s; }
    void setAssignee(const std::string& name){ assignee = name; }
    void setPriority(int p)                  { priority = (p >= 1 && p <= 5) ? p : PRIORITY_MEDIUM; }

    // ── Helpers ────────────────────────────────────────────────────────────
    std::string getStatusString() const {
        switch (status) {
            case IssueStatus::ENQUEUED:  return "ENQUEUED";
            case IssueStatus::CLAIMED:   return "CLAIMED";
            case IssueStatus::RUNNING:   return "RUNNING";
            case IssueStatus::COMPLETED: return "COMPLETED";
            case IssueStatus::BLOCKED:   return "BLOCKED";
        }
        return "UNKNOWN";
    }

    std::string getPriorityLabel() const {
        switch (priority) {
            case 1: return "CRITICAL";
            case 2: return "HIGH";
            case 3: return "MEDIUM";
            case 4: return "LOW";
            case 5: return "MINIMAL";
        }
        return "MEDIUM";
    }

    // Parse a status string back to enum (used by persistence layer)
    static IssueStatus parseStatus(int code) {
        switch (code) {
            case 0: return IssueStatus::ENQUEUED;
            case 1: return IssueStatus::CLAIMED;
            case 2: return IssueStatus::RUNNING;
            case 3: return IssueStatus::COMPLETED;
            case 4: return IssueStatus::BLOCKED;
            default: return IssueStatus::ENQUEUED;
        }
    }

    static IssueStatus parseStatusFromString(const std::string& s) {
        if (s == "ENQUEUED")  return IssueStatus::ENQUEUED;
        if (s == "CLAIMED")   return IssueStatus::CLAIMED;
        if (s == "RUNNING")   return IssueStatus::RUNNING;
        if (s == "COMPLETED") return IssueStatus::COMPLETED;
        if (s == "BLOCKED")   return IssueStatus::BLOCKED;
        throw std::invalid_argument("Unknown status string: " + s);
    }
};

#endif // ISSUE_HPP