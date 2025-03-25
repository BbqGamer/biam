#include "qap.h"
#include "random.h"
#include <stdio.h>
typedef int (*evalfunc)(int *, struct QAP *, int *, int *);

void execute_test(evalfunc search, struct QAP *instance, int *solution,
                  char *name) {
  int K = 10;
  int sum = 0;
  int max = -1;
  int min = 1000000000;
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

  struct QAP instance;
  read_instance(argv[1], &instance);
  printf("Loaded file! \n\n");

  int solution[MAX_QAP_SIZE];

  execute_test(randomsearch, &instance, solution, "Random Search");
  execute_test(randomwalk, &instance, solution, "Random Walk");
  execute_test(heuristic, &instance, solution, "Heuristic");
  execute_test(localsearchgreedy, &instance, solution, "Greedy");
  execute_test(localsearchsteepest, &instance, solution, "Steepest");
}
