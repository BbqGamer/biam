#include "qap.h"
#include "random.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
typedef void (*evalfunc)(struct QAP *, struct QAP_results *);

void execute_test(evalfunc search, struct QAP *instance, char *name) {
  int K = 10;
  int sum = 0;
  int max = INT_MIN, min = INT_MAX;
  int score, sum_evaluated = 0;
  int sum_steps = 0;

  struct QAP_results res;

  for (int _ = 0; _ < K; _++) {
    res.evaluated = 0;
    res.steps = 0;

    srand(_ + 2);
    random_permutation(res.solution, instance->n);

    search(instance, &res);
    score = evaluate_solution(res.solution, instance);
    assert(res.score == score);
    sum += score;
    sum_evaluated += res.evaluated;
    sum_steps += res.steps;
    if (score > max) {
      max = score;
    } else if (score < min) {
      min = score;
    }
  }
  printf("%s : %.2f (%d - %d)\n", name, (float)sum / K, min, max);
  printf("Evaluated solutions: %.2f Steps: %.2f \n\n", (float)sum_evaluated / K,
         (float)sum_steps / K);
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
  printf("Loaded .dat file! \n");
  instance.timeout_ms = 10;

  struct QAP_results res;
  if (read_solution(sln_path, instance.n, &res)) {
    printf("Loaded .sln file!\n\n");
    printf("Optimal score: %d\n\n", res.score);
  }

  execute_test(randomsearch, &instance, "Random Search");
  execute_test(randomwalk, &instance, "Random Walk");
  execute_test(heuristic, &instance, "Heuristic");
  execute_test(localsearchgreedy, &instance, "Greedy");
  execute_test(localsearchsteepest, &instance, "Steepest");
}
