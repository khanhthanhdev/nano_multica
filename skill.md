# Tool Skill: MicroMultica Orchestration Engine

## Context & Purpose
You are an AI Coding Agent acting as a technical team member. You coexist on a codebase managed by `multica`, a headless CLI state-machine tracking issues, statuses, and agent assignments. You must interact with this system to claim, update, and track your work.

## Installation Requirement
Ensure the global binary `multica` is accessible in your environment. If missing, alert the user or run the install script.

## CLI Command Reference
Always parse system results via stdout JSON. Do not guess issue states.

| Command Syntax | Expected Use Case | Output Format |
| :--- | :--- | :--- |
| `multica --create-issue "<title>" "<desc>" "<tag>" [priority]` | Call this when you identify a new bug, refactor goal, or feature request. Priority is 1 (Critical) to 5 (Minimal). | JSON object containing `"status":"success"` |
| `multica --poll-next` | Call this to advance the state machine. It handles transitions (`ENQUEUED` -> `CLAIMED` -> `RUNNING`). | JSON object detailing the `event` and `status` |
| `multica --list-json` | Call this to read the current Kanban board layout before deciding your next move. | JSON array of all tracked issues |
| `multica --list-agents` | Call this to list registered agents with performance statistics (Gemini-Advanced, Cursor-Composer, Claude-3.5). | JSON array of all registered agents |
| `multica --assign-issue <id> <agent_name>` | Call this to manually override automatic allocation and assign an agent. | JSON object detailing the `event` and `status` |
| `multica --update-status <id> <STATUS>` | Call this to manually force a state transition (e.g. `COMPLETED`, `BLOCKED`). | JSON object detailing the `event` and `status` |

## Operational Lifecycle Workflow
When assigned to a project repository containing `multica`, you must execute the following loop:

1. **Read Board State:** Run `multica --list-json` to see outstanding work.
2. **Process Tasks:** Look for issues where `"status": "RUNNING"` and `"assignee"` matches your current identity/profile.
3. **Trigger Updates:** If an issue is stuck in `ENQUEUED` or `CLAIMED`, run `multica --poll-next` to step the engine forward until your task is marked `RUNNING`.
4. **Handle Blockers:** If your code compilation fails or you encounter a fatal dependency ambiguity, intentionally append the string `"CRASH"` to your logging context or step metrics so the engine shifts the state to `BLOCKED`, alerting human maintainers.

## JSON Error Handling Boundaries
* If the CLI returns Exit Code `1`, your input arguments are malformed or a parameter validation failed. Check your string quotes and flag format.
* If the CLI returns Exit Code `2`, a structural workspace save/load data fault has occurred. Stop execution and notify the user.
