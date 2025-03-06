BUILD_DIR := build
TEST_RANDOM := $(BUILD_DIR)/test_random
BENCHMARK := $(BUILD_DIR)/benchmark

all: $(TEST_RANDOM) $(BENCHMARK)

$(TEST_RANDOM): src/random.c src/test_random.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@

$(BENCHMARK): src/random.c src/benchmark.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
