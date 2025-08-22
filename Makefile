# Makefile for Queue Entropy Analysis
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O3 -I include -pthread
SRCDIR = src
TESTDIR = tests
BUILDDIR = build

# Source files (exclude performance test from main build)
SOURCES = $(filter-out $(SRCDIR)/performance_test.cpp, $(wildcard $(SRCDIR)/*.cpp))
TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp)

# Executables
MAIN_EXEC = market_entropy_analyzer
TEST_EXECS = $(TEST_SOURCES:$(TESTDIR)/%.cpp=test_%)

# Default target
all: $(MAIN_EXEC)

# Main executable
$(MAIN_EXEC): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Test executables
test_%: $(TESTDIR)/%.cpp $(filter-out $(SRCDIR)/main.cpp, $(SOURCES))
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build all tests
tests: $(TEST_EXECS)

# Run all tests
test: tests
	@echo "Running all tests..."
	@for test in $(TEST_EXECS); do \
		echo "Running $$test..."; \
		./$$test; \
		echo ""; \
	done

# Run specific test
test-queue: test_test_queue_edge_cases
	./test_test_queue_edge_cases

test-entropy: test_test_entropy_edge_cases
	./test_test_entropy_edge_cases

test-pipeline: test_test_pipeline_edge_cases
	./test_test_pipeline_edge_cases

test-market: test_test_market_simulation
	./test_test_market_simulation

# Performance test
perf: test_test_market_simulation
	./test_test_market_simulation

# Clean build artifacts
clean:
	rm -f $(MAIN_EXEC) $(TEST_EXECS)
	rm -rf $(BUILDDIR)
	rm -f *.o *.so *.a

# Install (copy to system bin)
install: $(MAIN_EXEC)
	cp $(MAIN_EXEC) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(MAIN_EXEC)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(MAIN_EXEC)

# Release build
release: CXXFLAGS += -DNDEBUG
release: $(MAIN_EXEC)

# Help
help:
	@echo "Available targets:"
	@echo "  all        - Build main executable"
	@echo "  tests      - Build all test executables"
	@echo "  test       - Run all tests"
	@echo "  test-queue - Run queue edge case tests"
	@echo "  test-entropy - Run entropy edge case tests"
	@echo "  test-pipeline - Run pipeline edge case tests"
	@echo "  test-market - Run market simulation tests"
	@echo "  perf       - Run performance tests"
	@echo "  clean      - Remove build artifacts"
	@echo "  debug      - Build with debug symbols"
	@echo "  release    - Build optimized release version"
	@echo "  install    - Install to system"
	@echo "  uninstall  - Remove from system"
	@echo "  help       - Show this help"

.PHONY: all tests test test-queue test-entropy test-pipeline test-market perf clean debug release install uninstall help
