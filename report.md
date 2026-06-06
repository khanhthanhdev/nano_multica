# MicroMultica — Final Project Report

Upon completing the project, our team has submitted this comprehensive technical report. The report logically documents our software development process and includes the following key sections:

---

## 1. Overall Flowchart

The high-level execution flow of our headless, non-interactive C++ command-line application from Start to Exit is illustrated in the diagram below:

```mermaid
graph TD
    A([Start: ./multica argv]) --> B[Initialize MulticaWorkspace]
    B --> C[Construct IssueBoard & Load State from Disk]
    C --> D[Instantiate and Register Polymorphic Agents]
    D --> E[Restore Historic Agent Performance Stats]
    E --> F[Construct CommandRouter]
    F --> G{Route Action Flag}
    
    G -->|--help| H[Print Help Guide]
    G -->|--create-issue| I[Validate & Create Issue]
    G -->|--poll-next| J[Execute Single Lifecycle Mutation Step]
    G -->|--list-json| K[Serialize & Print Board to stdout]
    G -->|--list-agents| L[Serialize & Print Agents to stdout]
    G -->|--assign-issue| M[Override & Assign Agent to Issue]
    G -->|--update-status| N[Force-Update Status Code]
    G -->|Invalid Flag| O[Throw std::invalid_argument]
    
    H --> P[Scope Exit: IssueBoard Destructs & Saves State to Disk]
    I --> P
    J --> P
    K --> P
    L --> P
    M --> P
    N --> P
    O --> Q[Catch Block]
    
    P --> R([Exit Code 0: Success])
    
    Q -->|std::invalid_argument| S[Print JSON Error to stderr] --> T([Exit Code 1: Validation Fail])
    Q -->|std::exception| U[Print JSON Error to stderr] --> V([Exit Code 2: System Fail])
```

---

## 2. Class & Architecture Diagram

Our design adopts a clean, modular object structure. Below is the UML class diagram illustrating relationships (such as inheritance, composition, and dependency) and how our components interact:

```mermaid
classDiagram
    class main {
        +int argc
        +char* argv[]
    }
    
    class MulticaWorkspace {
        -std::string issuesFile
        -std::string agentsFile
        +load(vector~Issue~&, unordered_map~string, shared_ptr~Agent~~&) const
        +save(vector~Issue~&, unordered_map~string, shared_ptr~Agent~~&) const
        -loadIssues(vector~Issue~&) const
        -loadAgentStats(unordered_map~string, shared_ptr~Agent~~&) const
        -saveIssues(vector~Issue~&) const
        -saveAgentStats(unordered_map~string, shared_ptr~Agent~~&) const
    }
    
    class Issue {
        -int id
        -std::string title
        -std::string description
        -std::string tag
        -IssueStatus status
        -std::string assignee
        -int priority
        +getId() int
        +getTitle() string
        +getDescription() string
        +getTag() string
        +getStatus() IssueStatus
        +getAssignee() string
        +getPriority() int
        +setStatus(IssueStatus)
        +setAssignee(string)
        +setPriority(int)
    }
    
    class Agent {
        <<Abstract>>
        #std::string name
        #std::string specialtyTag
        #int successCount
        #int totalCount
        +getName() string
        +getSpecialty() string
        +getSuccessCount() int
        +getTotalCount() int
        +getSuccessRate() double
        +recordResult(bool success)
        +computeScore(Issue) double*
        +executeTask(Issue) bool*
    }
    
    class ClaudeAgent {
        +executeTask(Issue) bool
    }
    
    class CursorAgent {
        +executeTask(Issue) bool
    }
    
    class GeminiAgent {
        +computeScore(Issue) double
        +executeTask(Issue) bool
    }
    
    class IssueBoard {
        -std::vector~Issue~ issues
        -std::unordered_map~std::string, std::shared_ptr~Agent~~ agentRegistry
        -MulticaWorkspace workspace
        +registerAgent(shared_ptr~Agent~)
        +addIssue(string, string, string, int)
        +assignIssue(int, string)
        +updateIssueStatus(int, string)
        +processNextLifecycleStep()
        +printBoardJSON() const
        +printAgentsJSON() const
        -findIssueById(int) Issue*
        -findBestAgent(Issue) shared_ptr~Agent~
        -emitStateChange(int, string, string)
    }
    
    class CommandRouter {
        -std::vector~std::string~ args
        +route(IssueBoard&) const
        +printHelp()$
        -requireArg(size_t, string) const
        -parseIntArg(string, string)$
    }
    
    %% Relationships
    main ..> MulticaWorkspace : Instantiates
    main ..> IssueBoard : Instantiates
    main ..> CommandRouter : Instantiates & routes
    
    IssueBoard *-- MulticaWorkspace : Composition (owns)
    IssueBoard *-- Issue : Composition (contains list of)
    IssueBoard *-- Agent : Aggregation (holds registry of std::shared_ptr)
    
    Agent <|-- ClaudeAgent : Inheritance
    Agent <|-- CursorAgent : Inheritance
    Agent <|-- GeminiAgent : Inheritance
    
    CommandRouter ..> IssueBoard : Directs mutations on
```

---

## 3. Algorithms

Instead of raw code, the logic of our three core processes is documented below in structured pseudo-code:

### Algorithm 1: Headless CLI Routing & Exception Boundary

This logic handles parameter parsing, routes the execution to the state mutators, and safely translates exceptions into Unix exit codes.

```text
FUNCTION main(argc, argv)
    TRY
        Initialize MulticaWorkspace with paths ("multica_issues.dat", "multica_agents.dat")
        Initialize IssueBoard board with workspace
        
        // Dynamically instantiate polymorphic agents
        Register ClaudeAgent("Claude-3.5", "auth")
        Register CursorAgent("Cursor-Composer", "database")
        Register GeminiAgent("Gemini-Advanced", "frontend")
        
        Load historic agent stats from workspace agent file
        
        Initialize CommandRouter router with (argc, argv)
        router.route(board)
        
        // At scope exit, board destructor automatically runs and saves states to disk
        RETURN 0 (Success)
        
    CATCH std::invalid_argument e
        PRINT_TO_STDERR "{"status":"error", "type":"validation", "message":" + e.what + "}"
        RETURN 1 (Validation Error)
        
    CATCH std::exception e
        PRINT_TO_STDERR "{"status":"error", "type":"system_failure", "message":" + e.what + "}"
        RETURN 2 (Runtime/System Error)
    END TRY
END FUNCTION
```

### Algorithm 2: Deterministic Agent Allocation & Score Compounding

When allocating an issue to an agent automatically, we calculate a score for each registered agent to choose the optimal assignee.

```text
FUNCTION IssueBoard::findBestAgent(issue)
    IF agentRegistry is empty THEN
        RETURN nullptr
    END IF

    bestAgent = nullptr
    bestScore = -1.0

    FOR EACH (agentName, agentPtr) IN agentRegistry DO
        IF agentPtr is null THEN
            CONTINUE
        END IF
        
        // Calculate affinity & metrics score
        score = 0.0
        IF agentPtr.specialtyTag == issue.tag THEN
            score = score + 10.0 // Specialty match bonus
        END IF
        
        // Add performance factor (between 0.0 and 5.0)
        score = score + (agentPtr.getSuccessRate() * 5.0)
        
        // Add priority urgency weight (converts 1..5 priority to 5..1 added weight)
        score = score + (6.0 - issue.priority)
        
        IF score > bestScore THEN
            bestScore = score
            bestAgent = agentPtr
        END IF
    END FOR

    RETURN bestAgent
END FUNCTION
```

### Algorithm 3: Life-cycle Mutation Process (`processNextLifecycleStep`)

Steps a single eligible issue forward through the state machine.

```text
FUNCTION IssueBoard::processNextLifecycleStep()
    processedAny = false

    FOR EACH issue IN issues DO
        // Transition 1: ENQUEUED -> CLAIMED
        IF issue.status == ENQUEUED THEN
            bestAgent = findBestAgent(issue)
            IF bestAgent is null THEN
                PRINT_TO_STDERR "Warning: No agents registered"
                CONTINUE
            END IF
            
            issue.assignee = bestAgent.name
            issue.status = CLAIMED
            EmitJSONStateChangeEvent(issue.id, CLAIMED, bestAgent.name)
            processedAny = true
            BREAK // Only mutate one issue per poll cycle
            
        // Transition 2: CLAIMED -> RUNNING
        ELSE IF issue.status == CLAIMED THEN
            issue.status = RUNNING
            EmitJSONStateChangeEvent(issue.id, RUNNING, issue.assignee)
            processedAny = true
            BREAK
            
        // Transition 3: RUNNING -> COMPLETED or BLOCKED
        ELSE IF issue.status == RUNNING THEN
            agentPtr = agentRegistry.find(issue.assignee)
            IF agentPtr not found OR agentPtr is null THEN
                issue.status = BLOCKED
                EmitJSONStateChangeEvent(issue.id, BLOCKED)
                processedAny = true
                BREAK
            END IF
            
            // Runtime Polymorphism: execute concrete agent logic
            success = agentPtr.executeTask(issue)
            agentPtr.recordResult(success) // Update success stats
            
            IF success THEN
                issue.status = COMPLETED
                EmitJSONStateChangeEvent(issue.id, COMPLETED, issue.assignee)
            ELSE
                issue.status = BLOCKED
                EmitJSONStateChangeEvent(issue.id, BLOCKED, issue.assignee)
            END IF
            processedAny = true;
            BREAK
        END IF
    END FOR

    IF processedAny is false THEN
        PRINT_TO_STDOUT "{"status":"idle", "message":"No actionable issues found."}"
    END IF
END FUNCTION
```

---

## 4. Implementation Details

Here are the key C++ snippets illustrating core object-oriented programming, smart memory management, and file persistence structures:

### A. Object-Oriented Programming (OOP) & Polymorphism

The system uses an abstract base class `Agent` representing a worker interface, and concrete classes `ClaudeAgent`, `CursorAgent`, and `GeminiAgent` implementing custom behavioral validation.

*From [Agent.hpp](file:///home/thanhkt/code/vinuni/nano_multica/include/Agent.hpp#L17-L73):*

```cpp
class Agent {
protected:
    std::string name;
    std::string specialtyTag;
    int successCount;
    int totalCount;

public:
    Agent(std::string n, std::string tag, int successes = 0, int total = 0)
        : name(std::move(n)), specialtyTag(std::move(tag)),
          successCount(successes), totalCount(total) {}

    virtual ~Agent() = default;

    // Pure virtual method forcing concrete subclasses to override behavior
    virtual bool executeTask(const Issue& issue) = 0;
    
    // Virtual score calculation supporting customization (overriding)
    virtual double computeScore(const Issue& issue) const {
        double score = 0.0;
        if (specialtyTag == issue.getTag()) score += 10.0;
        score += getSuccessRate() * 5.0;
        score += static_cast<double>(6 - issue.getPriority());
        return score;
    }
};
```

Subclasses implement custom rules. For example, `ClaudeAgent` fails if the word `CRASH` is in the description:

*From [Agent.hpp](file:///home/thanhkt/code/vinuni/nano_multica/include/Agent.hpp#L81-L93):*

```cpp
class ClaudeAgent : public Agent {
public:
    using Agent::Agent;

    bool executeTask(const Issue& issue) override {
        // Simulates custom engine behavior
        if (issue.getDescription().find("CRASH") != std::string::npos) {
            return false; // Triggers BLOCKED status
        }
        return true;
    }
};
```

### B. Dynamic Memory Management & Smart Pointers

Memory leaks are prevented by holding agents in an `unordered_map` of `std::shared_ptr<Agent>`. Instantiation is completed cleanly with `std::make_shared`.

*From [main.cpp](file:///home/thanhkt/code/vinuni/nano_multica/src/main.cpp#L31-L41):*

```cpp
// Dynamically allocate polymorphic agents via memory-safe shared_ptr
auto claudeAgent = std::make_shared<ClaudeAgent>("Claude-3.5",     "auth");
auto cursorAgent = std::make_shared<CursorAgent>("Cursor-Composer", "database");
auto geminiAgent = std::make_shared<GeminiAgent>("Gemini-Advanced", "frontend");

board.registerAgent(claudeAgent);
board.registerAgent(cursorAgent);
board.registerAgent(geminiAgent);
```

### C. File I/O and State Persistence

Workspace configurations are loaded and saved using text file serialization. Fields are separated using a pipe (`|`) delimiter.

*From [MulticaWorkspace.cpp](file:///home/thanhkt/code/vinuni/nano_multica/src/MulticaWorkspace.cpp#L95-L113):*

```cpp
void MulticaWorkspace::saveIssues(const std::vector<Issue>& issues) const {
    std::ofstream file(issuesFile, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "{\"status\":\"error\",\"source\":\"MulticaWorkspace::saveIssues\","
                  << "\"message\":\"Failed to open issues file for writing\"}\n";
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
```

---

## 5. Testing & Validation

Our integration test suite triggers normal operations, exception routing, polymorphic overrides, and file persistence.

### Test Matrix

| Test Case ID | Scenario / Feature Tested | Terminal Execution Syntax | Expected Behavior | Actual stdout / stderr Output | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-01** | Direct Issue Insertion | `./multica --create-issue "Fix Database Index" "Optimize query speeds" "database"` | Appends issue to database, prints success JSON. | `{"status":"success","message":"Issue created","id":1,"title":"Fix Database Index","tag":"database","priority":"MEDIUM"}` | **PASS** |
| **TC-02** | State Mutation (ENQUEUED $\rightarrow$ CLAIMED) | `./multica --poll-next` | Assigns highest score agent (`Cursor-Composer` via specialty match) and claims task. | `{"event":"state_change","issue":1,"status":"CLAIMED","agent":"Cursor-Composer"}` | **PASS** |
| **TC-03** | State Mutation (CLAIMED $\rightarrow$ RUNNING) | `./multica --poll-next` | Transitions issue from claimed to active execution status. | `{"event":"state_change","issue":1,"status":"RUNNING","agent":"Cursor-Composer"}` | **PASS** |
| **TC-04** | Complete State JSON Export | `./multica --list-json` | Exports the full Kanban board state as raw JSON array. | `[{"id":1,"title":"Fix Database Index","tag":"database","priority":"MEDIUM","priority_val":3,"status":"RUNNING","assignee":"Cursor-Composer"}]` | **PASS** |
| **TC-05** | Input Exception Boundary (Missing Field) | `./multica --create-issue "Incomplete Task"` | Catches invalid argument counts, returns Exit Code 1. | `{"status":"error","type":"validation","message":"Missing required argument. Usage: --create-issue <title> <desc> <tag> [priority]"}` | **PASS** |
| **TC-06** | Polymorphic Blocker Path | Description contains keyword `"CRASH"` on a `"auth"` ticket polled to completion | `ClaudeAgent` executeTask returns false, transition to `BLOCKED`. | `{"event":"state_change","issue":1,"status":"BLOCKED","agent":"Claude-3.5"}` | **PASS** |
| **TC-07** | Data Persistence Integrity | Check serialized content after running TC-01 to TC-03 | File contains exact fields matching pipe-delimited schema. | `1|Fix Database Index|Optimize query speeds|database|2|Cursor-Composer|3` | **PASS** |

---

## 6. Requirement Mapping

This section explicitly maps the technical requirements of the assignment to the file structures and line ranges of our C++ application:

| Requirement Category | Specific Implementation Detail | Target Code Reference |
| :--- | :--- | :--- |
| **1. Group Code Volume** | Safe conversion of C-style arguments, exception traps, deterministic assignment scoring formulas, and disk database deserialization. | • [CommandRouter.cpp](file:///home/thanhkt/code/vinuni/nano_multica/src/CommandRouter.cpp) (~150 lines)<br>• [IssueBoard.cpp](file:///home/thanhkt/code/vinuni/nano_multica/src/IssueBoard.cpp) (~250 lines)<br>• [MulticaWorkspace.cpp](file:///home/thanhkt/code/vinuni/nano_multica/src/MulticaWorkspace.cpp) (~150 lines)<br>• [main.cpp](file:///home/thanhkt/code/vinuni/nano_multica/src/main.cpp) (~80 lines) |
| **2. OOP Principles** | • Abstract Base Class definition with virtual destructor and pure virtual methods.<br>• Subclass inheritance (`ClaudeAgent`, `CursorAgent`, `GeminiAgent`).<br>• Dynamic runtime polymorphism via base pointer. | • [Agent.hpp:L17-L73](file:///home/thanhkt/code/vinuni/nano_multica/include/Agent.hpp#L17-L73) (Abstract class)<br>• [Agent.hpp:L81-L139](file:///home/thanhkt/code/vinuni/nano_multica/include/Agent.hpp#L81-L139) (Subclass declarations)<br>• [IssueBoard.cpp:L186-L196](file:///home/thanhkt/code/vinuni/nano_multica/src/IssueBoard.cpp#L186-L196) (Polymorphic call context) |
| **3. Smart Memory Management** | Allocation of dynamic agent workers via `std::make_shared`, registered in memory containers using `std::shared_ptr` to avoid raw heap errors. | • [main.cpp:L35-L41](file:///home/thanhkt/code/vinuni/nano_multica/src/main.cpp#L35-L41) (`std::make_shared` context)<br>• [IssueBoard.hpp:L29](file:///home/thanhkt/code/vinuni/nano_multica/include/IssueBoard.hpp#L29) (`std::shared_ptr` mapping storage) |
| **4. Advanced Structures & STL** | • Fast hash maps (`std::unordered_map`) for agent registry retrieval.<br>• Dynamically sized vectors (`std::vector`) for issues and arguments. | • [IssueBoard.hpp:L28-L29](file:///home/thanhkt/code/vinuni/nano_multica/include/IssueBoard.hpp#L28-L29) (`std::vector` and `std::unordered_map` declaration)<br>• [CommandRouter.hpp:L26](file:///home/thanhkt/code/vinuni/nano_multica/include/CommandRouter.hpp#L26) (`std::vector` for string routing) |
| **5. File I/O & Exception Handling** | • Input/output filestream readers and writer trunks (`std::ifstream`, `std::ofstream`).<br>• Try-Catch exception handlers in central entry point with exit signaling. | • [MulticaWorkspace.cpp:L17-L54](file:///home/thanhkt/code/vinuni/nano_multica/src/MulticaWorkspace.cpp#L17-L54) (File reading)<br>• [MulticaWorkspace.cpp:L95-L113](file:///home/thanhkt/code/vinuni/nano_multica/src/MulticaWorkspace.cpp#L95-L113) (File writing)<br>• [main.cpp:L20-L79](file:///home/thanhkt/code/vinuni/nano_multica/src/main.cpp#L20-L79) (General try-catch boundary) |
