# MicroMultica Skill Protocol for AI Coding Agents

This document defines the interface and protocol instructions for AI coding agents to integrate with and consume the **MicroMultica** CLI state engine.

---

## 1. System Integration

AI agents must invoke the compiled `multica` binary directly from the terminal without interactive inputs. The system persists state across separate runs in two local flat-file databases:
- `multica_issues.dat` (issue properties & statuses)
- `multica_agents.dat` (agent performance metrics & registries)

---

## 2. Standardized Agent Execution Protocol

### Step 1: Query Current Tasks
To discover active work items, invoke the list command:
```bash
./multica --list-json
```
**Expected Output:** A JSON array of issues:
```json
[
  {"id":1,"title":"Fix Auth","tag":"auth","priority":"HIGH","priority_val":2,"status":"ENQUEUED","assignee":"none"}
]
```

### Step 2: Push Lifecycle Steps (State transition)
To execute the automated routing state machine:
```bash
./multica --poll-next
```
Each invocation performs exactly **one** state transition globally. Agents should poll sequentially until they reach a terminal status (`COMPLETED` or `BLOCKED`).

#### State Transitions Map:
1. **`ENQUEUED` $\rightarrow$ `CLAIMED`**: Assigns the highest scoring agent based on specialty match, historic success rate, and issue priority.
2. **`CLAIMED` $\rightarrow$ `RUNNING`**: Marks the issue as active.
3. **`RUNNING` $\rightarrow$ `COMPLETED` | `BLOCKED`**: Executes simulated logic. If description contains `"CRASH"`, it transitions to `BLOCKED`.

### Step 3: Handle Exit Codes
AI agents must check the command's exit code to ensure operational success:
- **`0`**: Command completed successfully.
- **`1`**: Validation error (invalid priority, missing arguments, or unknown statuses). Check `stderr`.
- **`2`**: System-level failure or runtime error. Check `stderr`.

---

## 3. Custom Mutations (Escalations and Manual Assignment)

For custom tasks requiring manual intervention:

### Assign an Agent manually:
```bash
./multica --assign-issue <id> <agent_name>
```
*Valid agent names:* `"Claude-3.5"`, `"Cursor-Composer"`, `"Gemini-Advanced"`.

### Update Status manually:
```bash
./multica --update-status <id> <STATUS>
```
*Valid STATUS strings:* `ENQUEUED`, `CLAIMED`, `RUNNING`, `COMPLETED`, `BLOCKED`.
