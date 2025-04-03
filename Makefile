BUILD_DIR := build
LDLAGS := -lm
CFLAGS := -o3 -Wall -Wextra -pedantic -Iinclude -g
TEST_RANDOM := $(BUILD_DIR)/test_random
BENCHMARK := $(BUILD_DIR)/benchmark
TEST_QAP := $(BUILD_DIR)/test_qap

all: $(TEST_RANDOM) $(BENCHMARK) $(TEST_QAP) $(REPORT)

$(TEST_RANDOM): src/random.c src/test_random.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@

$(BENCHMARK): src/random.c src/qap.c src/benchmark.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@

$(TEST_QAP): src/test_qap.c src/qap.c src/random.c| $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@ $(LDLAGS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
