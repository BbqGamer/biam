BUILD_DIR := build
TEST_RANDOM := $(BUILD_DIR)/test_random

all: $(TEST_RANDOM)

$(TEST_RANDOM): src/random.c src/test_random.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
