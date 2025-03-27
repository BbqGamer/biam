#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_QAP_SIZE 256

struct QAP {
  int A[MAX_QAP_SIZE * MAX_QAP_SIZE];
  int B[MAX_QAP_SIZE * MAX_QAP_SIZE];
  int n;
  int timeout_ms;
};

struct QAP_results {
  int score;
  int solution[MAX_QAP_SIZE];
  int evaluated;
  int steps;
};

void read_instance(char *filename, struct QAP *qap);
int read_solution(char *filename, int n, struct QAP_results *res);
int evaluate_solution(int *sol, struct QAP *qap);
int get_delta(int *sol, int i, int j, struct QAP *qap);

void heuristic(struct QAP *qap, struct QAP_results *res);
void randomsearch(struct QAP *qap, struct QAP_results *res);
void randomwalk(struct QAP *qap, struct QAP_results *res);
void localsearchgreedy(struct QAP *qap, struct QAP_results *res);
void localsearchsteepest(struct QAP *qap, struct QAP_results *res);