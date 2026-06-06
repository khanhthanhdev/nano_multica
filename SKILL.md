---
name: nano-multica
description: |
  MicroMultica is a headless C++ CLI state-machine engine for AI agents to track issues, manage execution lifecycles, and coordinate multi-agent workflows.
  Use when the user asks about task tracking, issue management, agent coordination, or mentions multica, nano_multica, or MicroMultica.
  The agent should auto-install multica if not present, then use it for all issue/task operations.
---

# MicroMultica — Agent-Callable Issue Orchestration Engine

## What this skill does

Provides a complete CLI-based issue tracking and state machine (`multica`) that AI agents can call directly. It manages tasks through a lifecycle: `ENQUEUED → CLAIMED → RUNNING → COMPLETED/BLOCKED`, with JSON output for programmatic consumption.

## When to use

- The user asks about issue/task tracking in their project
- The user mentions `multica`, `nano_multica`, or MicroMultica
- You need to create, claim, update, or list issues
- You're coordinating work across multiple agents or sessions
- You detect a `multica_issues.dat` or `multica_agents.dat` file in the project

## When NOT to use

- The user is asking about a different issue tracker (GitHub Issues, Jira, Linear)
- The project does not use nano_multica

## Inputs needed

- None for basic operations; issue title/description/tag for creation

## Step-by-step procedure

### 1. Installation check

First, verify `multica` is available:

```bash
which multica 2>/dev/null || echo "NOT_INSTALLED"
```

If not installed, run the install script bundled with this skill (`scripts/install.sh`). It will:
- Clone the repo from `https://github.com/khanhthanhdev/nano_multica.git`
- Compile the binary with `make`
- Install to `/usr/local/bin` (needs sudo) or `~/.local/bin`

### 2. Available commands

| Command | Purpose | Output |
|---------|---------|--------|
| `multica --create-issue "<title>" "<desc>" "<tag>" [priority]` | Create a new issue | JSON `{"status":"success"}` |
| `multica --poll-next` | Advance state machine one step | JSON event + status |
| `multica --list-json` | List all issues as JSON array | JSON array |
| `multica --list-agents` | List registered agents with stats | JSON array |
| `multica --assign-issue <id> <agent_name>` | Manually assign an issue | JSON event + status |
| `multica --update-status <id> <STATUS>` | Force status transition | JSON event + status |

### 3. Operational workflow

1. **Read board:** `multica --list-json` to see all issues
2. **Find your work:** Look for issues with `"status": "RUNNING"` assigned to you
3. **Claim next:** If something is `ENQUEUED`, run `multica --poll-next` to advance
4. **Handle blockers:** If blocked, set status: `multica --update-status <id> BLOCKED`
5. **Mark complete:** `multica --update-status <id> COMPLETED`

### 4. Exit codes

- `0` = success
- `1` = invalid args
- `2` = system/logic error

## Validation / how to know we're done

- `multica --list-json` returns valid JSON with expected issue states
- Exit code is always `0` for successful operations

## Common failure modes

| Symptom | Cause | Fix |
|---------|-------|-----|
| Exit code 1 | Malformed arguments | Check quotes, flag spelling |
| Exit code 2 | Corrupt state files | Delete `multica_*.dat` and recreate |
| `command not found` | Not installed | Run install script |

## Registered agents

| Agent | Specialty | Failure condition |
|-------|-----------|-----------------|
| Claude-3.5 | auth | Description contains "CRASH" |
| Cursor-Composer | database | Empty title |
| Gemini-Advanced | frontend | Priority 1 + "UNVERIFIED" in description |
