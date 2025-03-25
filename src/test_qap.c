#include "qap.h"
#include "random.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>
typedef int (*evalfunc)(int *, struct QAP *, int *, int *);

void execute_test(evalfunc search, struct QAP *instance, int *solution,
                  char *name) {
  int K = 10;
  int sum = 0;
  int max = -1;
  int min = INT_MAX;
  int evaluated;
  int sum_evaluated = 0;
  int steps;
  int sum_steps = 0;
  for (int _ = 0; _ < K; _++) {
    evaluated = 0;
    steps = 0;

    srand(_ + 2);
    random_permutation(solution, instance->n);
    search(solution, instance, &evaluated, &steps);
    int result = evaluate_solution(solution, instance);
    sum += result;
    sum_evaluated += evaluated;
    sum_steps += steps;
    if (result > max) {
      max = result;
    } else if (result < min) {
      min = result;
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

  int score, solution[MAX_QAP_SIZE];
  read_solution(sln_path, instance.n, solution, &score);
  printf("Loaded .sln file!\n\n");
  printf("Optimal score: %d\n\n", score);

  execute_test(randomsearch, &instance, solution, "Random Search");
  execute_test(randomwalk, &instance, solution, "Random Walk");
  execute_test(heuristic, &instance, solution, "Heuristic");
  execute_test(localsearchgreedy, &instance, solution, "Greedy");
  execute_test(localsearchsteepest, &instance, solution, "Steepest");
}
