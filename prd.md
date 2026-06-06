# Product Requirements Document (PRD) — Simple Task Manager CLI (MicroMultica)

## 1. Project Overview & Scope
**MicroMultica** is a headless, command-driven command-line interface (CLI) designed to serve as the execution and state-tracking layer for external AI coding agents. The system allows an agent to register itself, retrieve issues from a backlog, assign issues dynamically based on specialty matching and historic performance, simulate tasks, and log performance counters.

The software is structured as a non-interactive console application. It loads state from disk, executes a single state-machine transition or query per invocation, outputs formatted JSON, and terminates with standard Unix exit codes. This allows for automated parsing and scripting.

---

## 2. Project Description and Architecture

### 2.1 Description
MicroMultica acts as an issue tracker and orchestrator specifically tailored for AI agents. By executing command flags, external workflows can create issues, query the status of the board, list agents, or trigger lifecycle mutations. The CLI provides a deterministic framework to assess which agent is best suited for a task, runs the task simulation (which can succeed or fail depending on agent capability and task descriptions), and saves all changes back to a persistent flat-file database.

### 2.2 Core Algorithms

#### 2.2.1 State-Machine Lifecycle Transition
The project centers on a finite state machine managing task execution. Issues progress through a linear sequence:
$$\text{ENQUEUED} \longrightarrow \text{CLAIMED} \longrightarrow \text{RUNNING} \longrightarrow \text{COMPLETED} \text{ or } \text{BLOCKED}$$

- **ENQUEUED $\rightarrow$ CLAIMED**: Triggered when the CLI polls for work. The system determines the optimal registered agent for the task using the *Score Compounding Algorithm*. The chosen agent is assigned to the issue, and the status changes to `CLAIMED`.
- **CLAIMED $\rightarrow$ RUNNING**: Indicates that execution has started.
- **RUNNING $\rightarrow$ COMPLETED / BLOCKED**: The assigned agent attempts to execute the task. Based on the agent's rules and task parameters, the run either succeeds (status: `COMPLETED`) or fails/blocks (status: `BLOCKED`). The agent's historical success counters are updated accordingly.

```
       +--------------+
       |   ENQUEUED   |
       +-------+------+
               |
               | (Score-based Allocation)
               v
       +--------------+
       |   CLAIMED    |
       +-------+------+
               |
               | (Start Execution)
               v
       +--------------+
       |   RUNNING    |
       +---+------+---+
           |      |
           |      +------------------+
           | (Success)               | (Failure / CRASH)
           v                         v
     +-----+--------+          +-----+--------+
     |  COMPLETED   |          |   BLOCKED    |
     +--------------+          +--------------+
```

#### 2.2.2 Deterministic Agent Allocation (Score Compounding)
When an issue is in the `ENQUEUED` state, it is assigned to the agent that achieves the highest affinity score. The score is calculated using three compounded factors:
1. **Specialty Match Bonus**: A $+10.0$ bonus is added if the agent's specialty tag matching the issue's domain tag (e.g., `auth`, `database`, `frontend`).
2. **Performance History**: A weight between $0.0$ and $5.0$, computed as:
   $$\text{Success Rate Score} = \text{Success Rate} \times 5.0$$
   where $\text{Success Rate} = \frac{\text{Success Count}}{\text{Total Count}}$ (defaults to $1.0$ if the agent has no history).
3. **Priority Urgency Weight**: More urgent issues escalate matching scores to route them to the best-performing agents:
   $$\text{Priority Weight} = 6.0 - \text{Priority}$$
   (where Priority is 1 for Critical and 5 for Minimal).

$$\text{Final Score} = \text{Specialty Match Bonus} + (\text{Success Rate} \times 5.0) + (6.0 - \text{Priority})$$

### 2.3 Architecture & Class Diagram
The software is designed using standard object-oriented patterns in C++.

```mermaid
classDiagram
    class main {
        +int argc
        +char* argv[]
    }
    
    class Workspace {
        <<Abstract>>
        +load(vector~Issue~&, unordered_map~string, Agent~&) const*
        +save(vector~Issue~&, unordered_map~string, Agent~&) const*
        +loadIssues(vector~Issue~&) const*
        +loadAgentStats(unordered_map~string, Agent~&) const*
        +saveIssues(vector~Issue~&) const*
        +saveAgentStats(unordered_map~string, Agent~&) const*
    }
    
    class MulticaWorkspace {
        -std::string issuesFile
        -std::string agentsFile
        +load(vector~Issue~&, unordered_map~string, Agent~&) const
        +save(vector~Issue~&, unordered_map~string, Agent~&) const
        +loadIssues(vector~Issue~&) const
        +loadAgentStats(unordered_map~string, Agent~&) const
        +saveIssues(vector~Issue~&) const
        +saveAgentStats(unordered_map~string, Agent~&) const
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
        -std::string name
        -std::string specialtyTag
        -int successCount
        -int totalCount
        +getName() string
        +getSpecialty() string
        +getSuccessCount() int
        +getTotalCount() int
        +getSuccessRate() double
        +recordResult(bool success)
        +computeScore(Issue) double
        +executeTask(Issue) bool
    }
    
    class IssueBoard {
        -std::vector~Issue~ issues
        -std::unordered_map~std::string, Agent~ agentRegistry
        -std::shared_ptr~Workspace~ workspace
        +registerAgent(Agent)
        +addIssue(string, string, string, int)
        +assignIssue(int, string)
        +updateIssueStatus(int, string)
        +processNextLifecycleStep()
        +printBoardJSON() const
        +printAgentsJSON() const
        -findIssueById(int) Issue*
        -findBestAgent(Issue) const Agent*
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
    main ..> Workspace : Instantiates (as MulticaWorkspace)
    main ..> IssueBoard : Instantiates
    main ..> CommandRouter : Instantiates & routes
    
    IssueBoard o-- Workspace : Aggregation (holds std::shared_ptr)
    Workspace <|-- MulticaWorkspace : Inheritance (Polymorphism)
    IssueBoard *-- Issue : Composition (contains list of)
    IssueBoard *-- Agent : Composition (holds registry of Agent values)
    
    CommandRouter ..> IssueBoard : Directs mutations on
```

- **`Issue`**: Encapsulates a single work item, maintaining identification, description, status, current assignee, and priority.
- **`Agent`**: A concrete class representing a standard user/agent profile containing name, specialty tag, and historic success metrics. Task execution validation is dynamically routed based on specialty tags.
- **`Workspace`**: An abstract interface defining the contracts for loading/saving backlogs and agent profiles.
- **`MulticaWorkspace`**: A concrete workspace subclass implementing flat-file (pipe-delimited) load and save functionality.
- **`IssueBoard`**: The orchestrator representing the backlog and registry. It accepts a `std::shared_ptr<Workspace>` via dependency injection.
- **`CommandRouter`**: Parses command-line parameters, validates options, and routes calls to relevant methods on the board.

### 2.4 Expected Output
1. **Successful Execution**: Output formatted JSON containing the transition state changes or query details, terminating with exit code `0`.
2. **Validation/Arguments Error**: Output a structured error JSON detailing missing parameters or malformed inputs on `stderr`, terminating with exit code `1`.
3. **Runtime/System Error**: Output system failure information (such as file I/O blocks) on `stderr`, terminating with exit code `2`.

---

## 3. Development Tools and Resources

### 3.1 Libraries
The application relies strictly on standard C++ libraries to optimize compatibility, avoid runtime dependency bloat, and ensure safety:
- **`<iostream>`**: For writing structured JSON outputs to `stdout` and diagnostic logs or errors to `stderr`.
- **`<vector>`**: Dynamically allocated sequential container for lists of issues and command parameters.
- **`<unordered_map>`**: For $O(1)$ lookups when matching agent names to their polymorphic reference instances in the registry.
- **`<memory>`**: Essential for `std::shared_ptr` and `std::make_shared` to establish dynamic memory management and eliminate memory leaks.
- **`<fstream>`**: File input and output streams for reading/writing persistence files.
- **`<string>`**: Standard text representation, manipulation, and substring matching.
- **`<algorithm>`**: Utility functions for search boundaries and sorting operations.
- **`<stdexcept>`**: For uniform throwing of `std::invalid_argument` and `std::runtime_error` to catch at execution boundaries.

### 3.2 Frameworks
**No external application frameworks** are used. By relying on a raw C++ toolchain (with C++17 support) and standard compiler flags managed by a `Makefile`, the application compiles into a lean executable without dependencies, making it simple to package and install.

---

## 4. Technical Requirements

The implementation satisfies the academic criteria for a 4-student group project:

### 4.1 Code Volume
The system contains over 800 lines of functional C++ code (excluding blanks, standard imports, and comments). Modularity splits operations into distinct logic boundaries across files.

### 4.2 Object-Oriented Programming (OOP)
- **Classes & Encapsulation**: Data members in `Issue`, `Agent`, `IssueBoard`, and `MulticaWorkspace` are private/protected and manipulated via accessor/mutator methods.
- **Inheritance & Polymorphism**:
  - `Workspace` is an abstract base class with a virtual destructor and pure virtual load/save contracts.
  - `MulticaWorkspace` inherits from `Workspace` and overrides all load/save operations.
  - The `IssueBoard` orchestrator interacts with the workspace polymorphically through a base `std::shared_ptr<Workspace>` pointer.
- **Memory Management**:
  - Instantiation of the workspace is done dynamically using smart pointers: `std::shared_ptr<Workspace> workspace = std::make_shared<MulticaWorkspace>("multica_issues.dat", "multica_agents.dat")`.
  - The workspace interface is injected into the board, ensuring exception-safe memory tracking without raw pointers or memory leaks.

### 4.4 Advanced Data Structures & STL
- **Vectors (`std::vector`)**: Used for sequential ordering of active issues and flag parameters.
- **Hash Maps (`std::unordered_map`)**: Key-value maps (`std::string` to concrete `Agent` values) used for agent registration.
- **Streams**: Structured buffer reading and writing for flat files.

### 4.5 File I/O & Exception Handling
- State is serialized/deserialized automatically to and from files using `std::ifstream` and `std::ofstream`.
- Flat file records are parsed using a pipe-delimited parser (`|`).
- A centralized validation boundary protects the process via `try-catch` blocks in `main.cpp`, routing custom runtime failures to uniform JSON structures on `stderr`.

### 4.6 Modularity
Source code is organized into a clean folder structure containing separate interfaces (`.hpp`) and implementations (`.cpp`):
- `include/Agent.hpp` & `include/Issue.hpp`
- `include/IssueBoard.hpp` & `src/IssueBoard.cpp`
- `include/MulticaWorkspace.hpp` & `src/MulticaWorkspace.cpp`
- `include/CommandRouter.hpp` & `src/CommandRouter.cpp`
- `src/main.cpp`
- `Makefile`

---

## 5. Final Project Report Structure

The final project report must follow this logical layout to document the completed software:

### 5.1 Overall Flowchart
A diagram detailing execution flow from CLI entry, through loading state files, routing command arguments, executing mutations, writing back to disk, and returning exit codes.

### 5.2 Class & Architecture Diagram
A comprehensive UML Class Diagram visualizing encapsulation, class data members, functions, inheritance relationships, and ownership rules (such as compositions and aggregations).

### 5.3 Algorithms
Detailed pseudo-code outlines describing:
1. Entry routing and exception boundary operations.
2. The score compounding calculation rules.
3. The state transition logic for advancing issue lifecycles.

### 5.4 Implementation Details
Code extracts demonstrating OOP principles, inheritance, polymorphic bindings, smart pointer declarations, file streams, and delimiters, alongside concise explanations.

### 5.5 Testing & Validation
A complete test case table validating success scenarios, failure scenarios, invalid flag exceptions, validation failures, boundary states (like `CRASH` indicators or description limits), and file parsing checks.

### 5.6 Requirement Mapping
A reference index mapping the six project requirements to specific source files and line number spans.