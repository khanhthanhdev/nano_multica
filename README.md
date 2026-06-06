# MicroMultica — Agent-Callable CLI Orchestration Engine

A headless C++ state-machine engine for managing AI agent issue execution lifecycles.

## Skills

This repo publishes an [Agent Skill](https://skills.sh) for AI coding assistants:

### [`nano-multica`](./SKILL.md)

Instructions any AI agent can follow to auto-install and use the `multica` CLI for issue/task tracking.

### Install the skill

```bash
npx skills add khanhthanhdev/nano_multica
```

Or install just this specific skill:

```bash
npx skills add khanhthanhdev/nano_multica --skill nano-multica
```

### Usage (without the skill)

```bash
make clean && make          # build
./multica --list-json       # view issues
./multica --create-issue "Fix bug" "description" "tag" 2
./multica --poll-next       # advance state machine
```

## Build

```bash
make
```

Requires `g++` with C++17 support.

## Project structure

| Path | Description |
|------|-------------|
| `src/` | C++ source files |
| `include/` | C++ headers |
| `SKILL.md` | Agent Skill definition |
| `install.sh` | System install script |
| `multica_issues.dat` | Issue state (auto-created) |
| `multica_agents.dat` | Agent registry (auto-created) |

## Registered agents

| Agent | Specialty | Failure mode |
|-------|-----------|-------------|
| Claude Code | auth | Description contains "CRASH" |
| Github Copilot | database | Empty title (malformed task) |
| Codex | frontend | Priority 1 + "UNVERIFIED" in description |
| Opencode | general | None (always succeeds) |
| Pi | research | Description > 500 chars |
