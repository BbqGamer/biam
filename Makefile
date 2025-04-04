BUILD_DIR := build
LDLAGS := -lm
CFLAGS := -Ofast -march=native -flto -funroll-loops -fprefetch-loop-arrays -fno-exceptions -fopenmp -Wall -Wextra -pedantic -Iinclude -g
TEST_RANDOM := $(BUILD_DIR)/test_random
TEST_QAP := $(BUILD_DIR)/test_qap

all: $(TEST_RANDOM) $(BENCHMARK) $(TEST_QAP) $(REPORT)

$(TEST_RANDOM): src/random.c src/test_random.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@

$(TEST_QAP): src/test_qap.c src/qap.c src/random.c| $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@ $(LDLAGS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
