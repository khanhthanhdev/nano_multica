#!/bin/bash

# Colors for clear CLI test reporting
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# Ensure the executable exists
if [ ! -f "./multica" ]; then
    echo -e "${RED}Error: ./multica binary not found. Please compile the project first using 'make'.${NC}"
    exit 1
fi

echo "=== Starting MicroMultica Workflow Integration Test ==="

# Clean up any past test data files. 
# In our implementation, we use "multica_issues.dat" and "multica_agents.dat" 
# to split issue storage and agent performance records.
rm -f multica_issues.dat multica_agents.dat multica_workspace.dat

# Test Case 1: Issue Insertion
echo "Test 1: Creating a new issue..."
OUTPUT1=$(./multica --create-issue "Fix Database Index" "Optimize query speeds" "database")
echo "$OUTPUT1"
if [[ $OUTPUT1 == *"\"status\":\"success\""* ]]; then
    echo -e "${GREEN}[PASS] Issue creation successful.${NC}"
else
    echo -e "${RED}[FAIL] Issue creation failed.${NC}"
    exit 1
fi

# Test Case 2: State Machine Transition (ENQUEUED -> CLAIMED)
echo -e "\nTest 2: Polling for agent assignment..."
OUTPUT2=$(./multica --poll-next)
echo "$OUTPUT2"
if [[ $OUTPUT2 == *"\"status\":\"CLAIMED\""* && $OUTPUT2 == *"\"agent\":\"Cursor-Composer\""* ]]; then
    echo -e "${GREEN}[PASS] Correct agent claimed the specialized task.${NC}"
else
    echo -e "${RED}[FAIL] State machine failed to claim task.${NC}"
    exit 1
fi

# Test Case 3: State Machine Transition (CLAIMED -> RUNNING)
echo -e "\nTest 3: Advancing issue to RUNNING..."
OUTPUT3=$(./multica --poll-next)
echo "$OUTPUT3"
if [[ $OUTPUT3 == *"\"status\":\"RUNNING\""* ]]; then
    echo -e "${GREEN}[PASS] Issue successfully marked as active.${NC}"
else
    echo -e "${RED}[FAIL] Failed to transition to RUNNING state.${NC}"
    exit 1
fi

# Test Case 4: Complete State List Verification
echo -e "\nTest 4: Verifying final snapshot formatting..."
OUTPUT4=$(./multica --list-json)
echo "$OUTPUT4"
if [[ $OUTPUT4 == *"\"status\":\"RUNNING\""* && $OUTPUT4 == *"\"assignee\":\"Cursor-Composer\""* ]]; then
    echo -e "${GREEN}[PASS] End-to-end integration test passed successfully!${NC}"
else
    echo -e "${RED}[FAIL] Final board snapshot state invalid.${NC}"
    exit 1
fi

# Test Case 5: Error and Exception Handling (Bad inputs)
echo -e "\nTest 5: Verifying missing parameter validation..."
# Redirect stderr to a variable and capture the exit code
OUTPUT5=$(./multica --create-issue "Incomplete Task" 2>&1)
EXIT_CODE=$?
echo "Exit Code: $EXIT_CODE"
echo "Output: $OUTPUT5"
if [[ $EXIT_CODE -eq 1 && $OUTPUT5 == *"\"status\":\"error\""* ]]; then
    echo -e "${GREEN}[PASS] Missing parameters correctly caught with Exit Code 1.${NC}"
else
    echo -e "${RED}[FAIL] Missing parameters not handled correctly.${NC}"
    exit 1
fi

# Test Case 6: Polymorphic Blocker Path (Claude returns false if description contains 'CRASH')
echo -e "\nTest 6: Verifying polymorphic blocker path..."
# Clear issues database to test cleanly
rm -f multica_issues.dat multica_agents.dat

# 1. Create a problematic issue
echo "Creating issue containing 'CRASH' in description..."
./multica --create-issue "Fix Server" "The backend throws a severe CRASH error on boot" "auth" > /dev/null

# 2. Progress the issue through the cycle: ENQUEUED -> CLAIMED (by Claude-3.5) -> RUNNING -> BLOCKED
echo "Polling step 1 (ENQUEUED -> CLAIMED)..."
./multica --poll-next > /dev/null
echo "Polling step 2 (CLAIMED -> RUNNING)..."
./multica --poll-next > /dev/null
echo "Polling step 3 (RUNNING -> BLOCKED)..."
OUTPUT6=$(./multica --poll-next)
echo "$OUTPUT6"

if [[ $OUTPUT6 == *"\"status\":\"BLOCKED\""* ]]; then
    echo -e "${GREEN}[PASS] Polymorphic ClaudeAgent correctly failed and blocked the task.${NC}"
else
    echo -e "${RED}[FAIL] Task did not transition to BLOCKED state.${NC}"
    exit 1
fi

# Test Case 7: File Persistence (Data Integrity check)
echo -e "\nTest 7: Verifying file persistence structure..."
# Create a fresh issue and run it to RUNNING to match the test description
rm -f multica_issues.dat multica_agents.dat
./multica --create-issue "Fix Database Index" "Optimize query speeds" "database" > /dev/null
./multica --poll-next > /dev/null # CLAIMED
./multica --poll-next > /dev/null # RUNNING

# Since our workspace class writes to multica_issues.dat, check its contents.
# Note: Format is: id|title|description|tag|status_code|assignee|priority
# Status code 2 is RUNNING. Priority is default 3 (MEDIUM).
FILE_CONTENT=$(cat multica_issues.dat 2>/dev/null)
echo "Serialized content in multica_issues.dat:"
echo "$FILE_CONTENT"

EXPECTED_PATTERN="1|Fix Database Index|Optimize query speeds|database|2|Cursor-Composer|3"
if [[ "$FILE_CONTENT" == *"$EXPECTED_PATTERN"* ]]; then
    echo -e "${GREEN}[PASS] File persistence serialization verified successfully.${NC}"
else
    echo -e "${RED}[FAIL] File contents did not match expected structure.${NC}"
    exit 1
fi

echo -e "\n${GREEN}=== All Integration Tests Completed Successfully! ===${NC}"
exit 0
