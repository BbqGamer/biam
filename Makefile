BUILD_DIR := build
LDLAGS := -lm
CFLAGS := -Ofast -march=native -flto -funroll-loops -fprefetch-loop-arrays -fno-exceptions -fopenmp -Wall -Wextra -pedantic -Iinclude -g
TEST_RANDOM := $(BUILD_DIR)/test_utils
TEST_QAP := $(BUILD_DIR)/test_qap
TEST_TS := $(BUILD_DIR)/test_ts
TEST_SA := $(BUILD_DIR)/test_sa

all: $(TEST_RANDOM) $(BENCHMARK) $(TEST_QAP) $(TEST_TS) $(TEST_SA) $(REPORT)

$(TEST_RANDOM): src/utils.c src/test_utils.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@

$(TEST_QAP): src/test_qap.c src/qap.c src/utils.c src/algs.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@ $(LDLAGS)

$(TEST_TS): src/test_ts.c src/qap.c src/utils.c src/algs.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@ $(LDLAGS)

$(TEST_SA): src/test_sa.c src/qap.c src/utils.c src/algs.c | $(BUILD_DIR)
	gcc $(CFLAGS) $^ -o $@ $(LDLAGS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
