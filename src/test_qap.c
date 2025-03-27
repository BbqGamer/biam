#include "qap.h"
#include "random.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
typedef void (*evalfunc)(struct QAP *, struct QAP_results *);

#define K 10

float execute_test(evalfunc search, struct QAP *instance, char *name) {
  int sum = 0;
  int max = INT_MIN, min = INT_MAX;
  int score, sum_evaluated = 0;
  int sum_steps = 0;
  float cur_time, sum_time = 0;
  clock_t start, end;

  struct QAP_results res;

  fprintf(stderr, "%s: ", name);
  for (int _ = 0; _ < K; _++) {
    res.evaluated = 0;
    res.steps = 0;

    srand(_ + 2);
    random_permutation(res.solution, instance->n);

    start = clock();
    search(instance, &res);
    end = clock();

    score = evaluate_solution(res.solution, instance);
    assert(res.score == score);

    cur_time = (float)(end - start) / CLOCKS_PER_SEC * 1000;
    sum_time += cur_time;

    fprintf(stdout, "%s,%d,%.3f,%d,%d\n", name, score, cur_time, res.evaluated, res.steps);
    
    sum += score;
    sum_evaluated += res.evaluated;
    sum_steps += res.steps;
    if (score > max) {
      max = score;
    } else if (score < min) {
      min = score;
    }
  }
  fprintf(stderr, "%.2f (%d - %d)\n", (float)sum / K, min, max);
  fprintf(stderr, "avg time: %.3f ms, avg evals: %.2f, avg steps: %.2f \n\n",
     sum_time / K, (float)sum_evaluated / K, (float)sum_steps / K);
  return sum_time / K;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage %s [input file].dat", argv[0]);
    exit(1);
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
    fprintf(stderr, "Optimal score: %d\n\n", res.score);
  }

  float time_greedy = execute_test(localsearchgreedy, &instance, "G");
  float time_steepest = execute_test(localsearchsteepest, &instance, "S");
  instance.timeout_ms = (time_greedy + time_steepest) / 2;
  fprintf(stderr, "Timeout set to %.4f ms\n", instance.timeout_ms);

  execute_test(randomwalk, &instance, "RW");
  execute_test(randomsearch, &instance, "RS");
  execute_test(heuristic, &instance, "H");
}
