#include "qap.h"
#include "random.h"
typedef int (*evalfunc)(int *, struct QAP *);

void execute_test(evalfunc search, struct QAP *instance, int *solution,
                  char *name) {
                    int K = 1000;
  int sum = 0;
  int max = -1;
  int min = 1000000000;
  for (int _ = 0; _ < K; _++) {
    srand(_ + 2);
    random_permutation(solution, instance->n);
    search(solution, instance);
    int result = evaluate_solution(solution, instance);
    sum += result;
    if (result > max) {
      max = result;
    } else if (result < min) {
      min = result;
    }
  }
  printf("%s : %.2f (%d - %d)\n\n", name, (float)sum/K, min, max);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage %s [input file].dat", argv[0]);
    exit(1);
  }

  struct QAP instance;
  read_instance(argv[1], &instance);
  printf("Loaded file! \n");
  int solution[MAX_QAP_SIZE];

  int sum_random = 0;
  int sum_heuristic = 0;
  int max_random = -1;
  int max_heuristic = -1;
  int min_random = 1000000000;
  int min_heuristic = 1000000000;

  for (int _ = 0; _ < 10; _++) {
    srand(_ + 2);
    random_permutation(solution, instance.n);
    int result = evaluate_solution(solution, &instance);
    sum_random += result;
    if (result > max_random) {
      max_random = result;
    } else if (result < min_random) {
      min_random = result;
    }
  }
  printf("Random : %.2f (%d - %d)\n\n", (float)sum_random/10, min_random, max_random);
  for (int i = 0; i < instance.n; i++) {
    solution[i] = -1;
  }
  heuristic(solution, &instance);
  int h_score = evaluate_solution(solution, &instance);
  printf("Heuristic: %d\n", h_score);

  execute_test(localsearchgreedy, &instance, solution, "Greedy");
    execute_test(localsearchsteepest, &instance, solution, "Steepest");


  // random_permutation(solution, instance.n);
  // int ls_before = evaluate_solution(solution, &instance);
  // int ls_score = localsearchsteepest(solution, &instance);
  // int ls_score_sanity = evaluate_solution(solution, &instance);
  // printf("Before LS: %d\tAfter LS: %d (%d)\n", ls_before, ls_score,
  //        ls_score_sanity);

  // random_permutation(solution, instance.n);
  // ls_score = localsearchgreedy(solution, &instance);
  // ls_score_sanity = evaluate_solution(solution, &instance);
  // printf("Before LS: %d\tAfter LS: %d (%d)\n", ls_before, ls_score,
  //        ls_score_sanity);
}
