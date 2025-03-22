#include "qap.h"
#include "random.h"

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
  printf("Random : %d (%d - %d)\n\n", sum_random, min_random, max_random);
  for (int i = 0; i < instance.n; i++) {
    solution[i] = -1;
  }
  heuristic(solution, &instance);
  int h_score = evaluate_solution(solution, &instance);
  printf("Heuristic: %d", h_score);

  int a, b;
  random_pair(&a, &b, instance.n);
  int delta = get_delta(solution, a, b, &instance);
  printf("\nA: %d, B: %d\n", a, b);
  int temp = solution[a];
  solution[a] = solution[b];
  solution[b] = temp;
  int res_after_swap = evaluate_solution(solution, &instance);
  printf("Before: %d\tAfter: %d\n", h_score + delta, res_after_swap);

  random_permutation(solution, instance.n);
  int ls_before = evaluate_solution(solution, &instance);
  int ls_score = localsearch(solution, &instance);
  int ls_score_sanity = evaluate_solution(solution, &instance);
  printf("Before LS: %d\tAfter LS: %d (%d)\n", ls_before, ls_score, ls_score_sanity);
}
