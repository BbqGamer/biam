#include "qap.h"
#include "utils.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
typedef void (*evalfunc)(struct QAP *, struct QAP_results *);

float execute_test(evalfunc search, struct QAP *instance, char *name, int K) {
  long long sum = 0;
  int max = INT_MIN, min = INT_MAX;
  int start_score, score, sum_evaluated = 0;
  int sum_steps = 0;
  float cur_time, sum_time = 0;
  clock_t start, end;

  struct QAP_results res;
  int start_solution[MAX_QAP_SIZE];
  fprintf(stderr, "Running %s...\n", name);

  for (int _ = 0; _ < K; _++) {
    res.evaluated = 0;
    res.steps = 0;

    srand(_ + 2);
    random_permutation(res.solution, instance->n);
    memcpy(start_solution, res.solution, instance->n * sizeof(int));
    start_score = evaluate_solution(start_solution, instance);

    start = clock();
    search(instance, &res);
    end = clock();

    score = evaluate_solution(res.solution, instance);
    assert(res.score == score);

    cur_time = (float)(end - start) / CLOCKS_PER_SEC * 1000;
    sum_time += cur_time;

    fprintf(stdout, "%s,%d,%d,%.3f,%d,%d,", name, start_score, score, cur_time, res.evaluated, res.steps);
    for(int i = 0; i < instance->n - 1; i++) {
      printf("%d ", start_solution[i] + 1);
    }
    printf("%d,", start_solution[instance->n - 1] + 1);

    for(int i = 0; i < instance->n - 1; i++) {
      printf("%d ", res.solution[i] + 1);
    }
    printf("%d\n", res.solution[instance->n - 1] + 1);
    
    sum += score;
    sum_evaluated += res.evaluated;
    sum_steps += res.steps;
    if (score > max) {
      max = score;
    }
    if (score < min) {
      min = score;
    }
  }
  fprintf(stderr, "%.2f (%d - %d)\n", (float)sum / K, min, max);
  fprintf(stderr, "avg time: %.3f ms, avg evals: %.2f, avg steps: %.2f \n\n",
     sum_time / K, (float)sum_evaluated / K, (float)sum_steps / K);
  return sum_time / K;
}

int main(int argc, char *argv[]) {
  int K = 10;

  if(argc < 2) {
    fprintf(stderr, "Usage: %s <dat_file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char *dat_path = argv[1];
  char sln_path[1024];

  strncpy(sln_path, dat_path, sizeof(sln_path) - 1);
  sln_path[sizeof(sln_path) - 1] = '\0';

  char *dot = strrchr(sln_path, '.');
  if (dot && strcmp(dot, ".dat") == 0) {
    strcpy(dot, ".sln");
  } else {
    fprintf(stderr, "Input file must have a .dat extension.\n");
    exit(1);
  }

  struct QAP instance;
  read_instance(dat_path, &instance);
  fprintf(stderr, "Loaded .dat file! \n");
  instance.timeout_ms = 1000;

  struct QAP_results res;
  if (read_solution(sln_path, instance.n, &res)) {
    fprintf(stderr, "Loaded .sln file!\n\n");
    int eval_score = evaluate_solution(res.solution, &instance);
    if (eval_score != res.score) {
      fprintf(stderr, "Eval_score != res.score (%d != %d)", eval_score, res.score);
      return -1;
    }
    fprintf(stderr, "Optimal score: %d (sanity check: %d)\n\n", res.score, eval_score);
  }

  fprintf(stdout, "alg,start_score,score,time,evals,steps,starting,solution\n");
  execute_test(simulatedannealing, &instance, "simulatedannealing", K);
}
