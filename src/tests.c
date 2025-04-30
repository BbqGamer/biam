#include "utils.h"
#include "qap.h"
#include <glob.h>

#define REPS 20
#define ARR_SIZE 10

void test_evaluate() {
    glob_t pglob;
    glob("data/*.dat", 0, NULL, &pglob);
    size_t i;
    struct QAP qap;
    struct QAP_results res;
    for (i = 0; i < pglob.gl_pathc; i++) {
        read_instance(pglob.gl_pathv[i], &qap);
        if(read_solution(pglob.gl_pathv[i], qap.n, &res) != 1)
            continue;
        int score = evaluate_solution(res.solution, &qap);
        if (score != res.score) {
            fprintf(stderr, "%s: Evaluated score doesn't match .sln file (%d != %d)\n",
                  pglob.gl_pathv[i], score, res.score);
            for(int i = 0; i < qap.n; i++) {
                printf("%d ", res.solution[i]);
            }
            exit(1);
        }
    }
}

int main() {
  int i, j;
  int array[ARR_SIZE];

  printf("Generating random permutation of %d elements: \n", ARR_SIZE);
  for (i = 0; i < REPS; i++) {
    random_permutation(array, ARR_SIZE);
    for (j = 0; j < ARR_SIZE; j++) {
      printf("%d ", array[j]);
    }
    printf("\n");
  }
  printf("\n");

  printf("Generating random pairs:");
  for (i = 0; i < 10; i++) {
    int a, b;
    random_pair(&a, &b, ARR_SIZE);
    printf("%d %d\n", a, b);
  }

  test_evaluate();
}

// METRICS
// F_initial (score+permutation)
// F_final (score+permutation)
// time
// N_eval (how many deltas were calculated)
// N_sol (how many solutions were visited)
