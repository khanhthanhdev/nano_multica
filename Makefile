CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -Iinclude
SRC_DIR  := src
OBJ_DIR  := obj
TEST_DIR := tests
TEST_OBJ_DIR := $(OBJ_DIR)/test
TARGET   := multica
TEST_BIN := $(OBJ_DIR)/test_runner

SRCS     := $(wildcard $(SRC_DIR)/*.cpp)
OBJS     := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Project object files excluding main.o (test binaries don't include main.cpp)
PROJ_OBJS := $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ── Test target ────────────────────────────────────────────────────────────
test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(PROJ_OBJS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | $(TEST_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

$(TEST_OBJ_DIR):
	mkdir -p $(TEST_OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) /tmp/multica_test_*.dat multica_issues.dat multica_agents.dat

.PHONY: all clean test
