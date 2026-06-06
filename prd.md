To make this a CLI that an AI Coding Agent (like a Cursor Agent, Claude Engineer, or a custom Python script) can **call directly**, the program must shift from an *interactive* menu loop to a **headless, command-driven architecture**.

Instead of waiting for human keystrokes, the agent invokes your compiled binary directly via terminal commands with explicit flags (e.g., `./multica --create-issue "Fix Auth"` or `./multica --poll-next`). The program processes the command, updates the state files, prints structured logs or JSON to `stdout`, and exits immediately with an appropriate status code.

Here is the updated PRD adjusted for an **Agent-Executable CLI Engine**.

---

# Product Requirements Document (PRD)

## Project Title: MicroMultica: An Agent-Callable CLI Orchestration Engine

---

## 2. Project Description and Architecture

### Description

**MicroMultica** is a headless, non-interactive command-line tool built to serve as the local execution and state tracking layer for autonomous AI Coding Agents. Rather than relying on a human-facing UI, this CLI is designed to be executed directly by scripts or parent agent loops using standardized command line arguments (flags).

When an agent invokes the CLI to create an issue, claim a task, or update execution metrics, MicroMultica parses the command line parameters, loads the system state from local disk storage, performs the requested mutation, prints structured progress logs or raw JSON metadata to standard output (`stdout`), and returns a clean exit code (`0` for success, non-zero for failure).

### Algorithm

The core engine executes commands using two foundational backend processes:

1. **Command Routing and Flag Parsing:** The system translates `argv` inputs into specific execution contexts using an deterministic routing algorithm. It strictly validates flags, ensuring optional arguments match requirement criteria (e.g., preventing a task update if a non-existent status flag is passed) before initializing state mutations.
2. **Deterministic Agent Allocation & Score Compounding:** When a command requests an automatic allocation for an issue, the engine calculates a structural score for each registered agent based on their active capability array and historic task success rates. The agent with the highest compound score is automatically assigned, and the issue state transitions without human intervention.

### Architecture & Component Structure

The system uses clear multi-file modularization, abandoning interactive UI controllers in favor of strict command routers and file-persisted databases.

```
                      ┌───────────────────────┐
                      │         main          │ (Parses argv flags)
                      └───────────┬───────────┘
                                  │ Calls
                                  ▼
                      ┌───────────────────────┐
                      │     CommandRouter     │ (Validates & Routes Actions)
                      └───────────┬───────────┘
                                  │ Invokes
                                  ▼
                      ┌───────────────────────┐
                      │     IssueBoard        │ (State Mutator Engine)
                      └─────┬───────────┬─────┘
                            │           │
            Modifies Status │           │ References Profiles
                            ▼           ▼
                      ┌───────────┐┌───────────┐
                      │   Issue   ││   Agent   │ (Base Class)
                      └───────────┘└───────────┘

```

#### Class Definitions

* `Issue` (Class): Holds parameters for individual tasks (ID, Title, Status, Assignee, Priority Rating).
* `Agent` (Abstract Base Class / Derived Classes): Represents agent profiles. Defines how specific tool structures format outputs or handle execution simulated token limits.
* `IssueBoard` (Engine Class): Contains the collection of issues and agent registries. It is the core worker class that contains logic to add links, filter states, and run state transitions.
* `CommandRouter` (Controller Class): Contains the flag parsing maps. It takes `argc` and `argv`, maps inputs to internal methods, handles input exceptions, and routes payloads to the `IssueBoard`.
* `MulticaWorkspace` (Persistence Class): Reads/writes the board configuration directly to disk (`workspace.dat` or `issues.json`) before and after every execution slice.

### Expected Output

Because this is built for automated agents to call directly, all outputs must be highly predictable:

* **JSON Structured Outputs:** When queried with flags like `./multica --list-json`, the output must be parseable JSON string arrays so the calling agent can read them programmatically.
* **Standard Error Logging (`stderr`):** System bugs, missing files, or validation blockages must be routed to `std::cerr` rather than `std::cout`, separating errors from expected operational data.
* **Standardized Exit Codes:** Returns `0` for perfect command completion, `1` for invalid argument configurations, and `2` if an agent-blocking logic exception occurs.

---

## 3. Development Tools and Resources

### Libraries (Standard STL Only)

* `<iostream>` & `<fstream>`: Utilized to stream structural data to calling runtimes (`std::cout`, `std::cerr`) and pipeline internal configurations to disk.
* `<vector>`, `<string>`, & `<unordered_map>`: Used to track files, map active input commands to functional methods, and catalog active issue tracking arrays efficiently.
* `<memory>`: Enforces clean dynamic allocations using smart pointer wrappers (`std::unique_ptr` and `std::shared_ptr`) to eliminate explicit manual tracking overhead.
* `<algorithm>`: Powers sorting processes for tasks and streamlines command-line keyword matches.
* `<stdexcept>`: Isolates and captures parameter violations, triggering clean exit code signaling vectors back to the calling parent script.

### Build Configuration Tool

* **CMake (CMakeLists.txt):** Maps and links separate compilation units (`.cpp` and `.hpp` files) into a single optimized executable file target.

---

## Technical Requirements Checklist & Mapping Plan

To satisfy the academic workload for a **group of 4 students**, the framework splits core technical challenges directly across isolated boundaries:

| Technical Requirement | Specific Implementation Architecture | Target File Targets | Target Line Estimate |
| --- | --- | --- | --- |
| **1. Code Volume** | Writing comprehensive argument parsers, error boundary wrappers, state machines, and file translators naturally stretches code architecture past the target threshold. | `src/*.cpp`, `include/*.hpp` | ~900 LOC |
| **2. OOP Principles** | Extensively utilizes encapsulation across classes. Employs inheritance and polymorphism to execute target logic updates over varied agent archetypes. | `Agent.hpp`, `CommandRouter.cpp` | ~250 LOC |
| **3. Smart Memory** | Implements automated heap allocations via `std::shared_ptr` to safely build polymorphic object arrays without resource leak exposure. | `IssueBoard.hpp`, `main.cpp` | ~130 LOC |
| **4. Advanced Structures** | Leverages fast map collections (`std::unordered_map`) to index active task objects and speed up key flag query processing. | `IssueBoard.cpp` | ~170 LOC |
| **5. I/O & Exceptions** | Employs robust try-catch mechanisms to intercept processing data faults or configuration state errors, passing explicit exit codes back to standard systems. | `CommandRouter.cpp`, `main.cpp` | ~200 LOC |

---

## Final Project Report Framework

### 1. Overall Flowchart

The flowchart must map the immediate execution flow of a headless CLI utility:

* `Start` $\rightarrow$ Capture `argc` and `argv` context vectors.
* `Parse Input` $\rightarrow$ Evaluate target parameter flags. If flags are malformed $\rightarrow$ Write to `stderr` $\rightarrow$ Exit with Code 1.
* `Load Memory` $\rightarrow$ Parse state history database from disk storage.
* `Process Mutation` $\rightarrow$ Alter target task parameters or execute state change based on input command.
* `Commit Changes` $\rightarrow$ Re-write state history payload cleanly to local storage disk files.
* `Output Log` $\rightarrow$ Write response payload or structural JSON metrics to `stdout` $\rightarrow$ Exit Code 0.

### 2. Class & Architecture Diagram

A structural block diagram showing how `main.cpp` calls the `CommandRouter` interface class, which coordinates tasks via the `IssueBoard` container class holding base polymorphic configurations.

### 3. Algorithms Pseudo-code

#### Headless Command Parsing & Routing Engine

```text
FUNCTION main(argc, argv)
    IF argc < 2 THEN
        PRINT_TO_STDERR "Usage: ./multica --flag [options]"
        RETURN 1 (EXIT_FAILURE)
    END IF

    Initialize CommandRouter router
    TRY
        router.loadWorkspaceFromDisk()
        ActionFlag = argv[1]
        
        IF ActionFlag == "--create-issue" THEN
            ValidateParam(argv[2])
            ValidateParam(argv[3])
            ValidateParam(argv[4])
            router.executeCreateIssue(argv[2], argv[3], argv[4], argv[5] // optional)
        ELSE IF ActionFlag == "--poll-next" THEN
            router.executeAgentPollStep()
        ELSE IF ActionFlag == "--list-json" THEN
            router.printActiveBoardAsJSON()
        ELSE IF ActionFlag == "--list-agents" THEN
            router.printActiveAgentsAsJSON()
        ELSE IF ActionFlag == "--assign-issue" THEN
            router.executeManualAssignment(argv[2], argv[3])
        ELSE IF ActionFlag == "--update-status" THEN
            router.executeUpdateStatus(argv[2], argv[3])
        ELSE
            THROW InvalidArgumentException("Unknown CLI parameter flag")
        END IF
        
        router.saveWorkspaceToDisk()
        RETURN 0 (EXIT_SUCCESS)
        
    CATCH Exception e
        PRINT_TO_STDERR "ERROR: " + e.message()
        RETURN 2 (EXIT_LOGIC_ERROR)
    END TRY
END FUNCTION

```

### 4. Implementation Details

The project report demonstrates structural code requirements without human-facing user input logic (`std::cin`):

```cpp
// Architectural Snippet: Pure CLI Command Processor Target
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class CommandRouter {
private:
    std::vector<std::string> arguments;
public:
    CommandRouter(int argc, char* argv[]) {
        // Safe conversion of raw command array parameters into STL structures
        for (int i = 0; i < argc; ++i) {
            arguments.push_back(std::string(argv[i]));
        }
    }

    void process() {
        if (arguments.size() < 2) {
            throw std::runtime_error("Missing parameter flags. Use --help for usage details.");
        }
        
        if (arguments[1] == "--create-issue") {
            if (arguments.size() < 5) throw std::invalid_argument("Usage: --create-issue <title> <desc> <tag> [priority]");
            std::cout << "{\"status\":\"success\", \"message\":\"Issue created successfully\"}\n";
        }
    }
};

```

### 5. Testing & Validation Matrix

Because your software handles automated machine prompts, your test scripts must demonstrate robust parameter scanning:

| Test Case ID | Scenario / Feature Tested | Script Execution Syntax | Expected Standard Pipeline Behavior | Result |
| --- | --- | --- | --- | --- |
| **TC-01** | Direct Issue Insertion | `./multica --create-issue "Write DB Unit Test" "Create DB test cases" "database" 3` | Appends data node to local files; returns JSON payload on `stdout`, Exit Code 0. | Pass |
| **TC-02** | Automated State Advancement | `./multica --poll-next` | Polls unassigned tickets, links eligible workers, prints active mutation log strings. | Pass |
| **TC-03** | Missing Execution Fields | `./multica --create-issue "Incomplete Task"` | Captures empty parameter data boundary; routes error notice to `stderr`, Exit Code 1. | Pass |
| **TC-04** | Invalid Flow Invocation | `./multica --invalid-flag` | Handles unknown option safely using inner code catch traps; blocks program failure, Exit Code 1. | Pass |

---

## CLI Usage Guide

**MicroMultica** is an agent-executable headless CLI tool. To compile and run the engine, follow the guide below.

### 1. Compilation

Build the executable target using `make`:
```bash
make clean && make
```
This builds the `./multica` binary using standard C++17 compilers.

### 2. Available Commands and Syntaxes

#### 1. Injects a New Tracking Issue
* **Syntax:** `./multica --create-issue <title> <description> <tag> [priority]`
* **Arguments:** 
  * `<title>`: Short identifier for the issue.
  * `<description>`: Details about the task (include word `CRASH` to trigger simulated execution failures).
  * `<tag>`: Specialty tag matching agent specialties (`auth`, `database`, `frontend`).
  * `[priority]`: Optional priority level from `1` (Critical) to `5` (Minimal). Defaults to `3` (Medium).
* **Example:**
  ```bash
  ./multica --create-issue "Optimize DB Queries" "Fix slow indices" "database" 2
  ```

#### 2. Drives State-Machine Lifecycle Step
* **Syntax:** `./multica --poll-next`
* **Description:** Advances exactly one issue by one state transition step (ENQUEUED $\rightarrow$ CLAIMED $\rightarrow$ RUNNING $\rightarrow$ COMPLETED/BLOCKED). Automatically calculates compound scores to assign issues to the best available agent.
* **Example:**
  ```bash
  ./multica --poll-next
  ```

#### 3. Lists All Board Issues in JSON
* **Syntax:** `./multica --list-json`
* **Description:** Dumps all current issue models stored in `multica_issues.dat` as a structured JSON array.
* **Example:**
  ```bash
  ./multica --list-json
  ```

#### 4. Lists Registered Agents in JSON
* **Syntax:** `./multica --list-agents`
* **Description:** Streams out stats of registered agents (Gemini-Advanced, Cursor-Composer, Claude-3.5) including their specialty, success count, and rate.
* **Example:**
  ```bash
  ./multica --list-agents
  ```

#### 5. Manually Assign an Issue
* **Syntax:** `./multica --assign-issue <id> <agent_name>`
* **Description:** Overrides automatic allocation to claim a task manually for a specific agent.
* **Example:**
  ```bash
  ./multica --assign-issue 1 "Claude-3.5"
  ```

#### 6. Force Update Issue Status
* **Syntax:** `./multica --update-status <id> <STATUS>`
* **Description:** Forces the issue `<id>` to transition into the specified state (`ENQUEUED`, `CLAIMED`, `RUNNING`, `COMPLETED`, or `BLOCKED`).
* **Example:**
  ```bash
  ./multica --update-status 1 COMPLETED
  ```

### 3. Exit Code Structure
* **`0`**: Command successfully processed.
* **`1`**: Invalid arguments, missing parameters, or status validation errors.
* **`2`**: System-level failure or execution anomalies.


---