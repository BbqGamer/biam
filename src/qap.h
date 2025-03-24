#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_QAP_SIZE 256

struct QAP {
  int A[MAX_QAP_SIZE * MAX_QAP_SIZE];
  int B[MAX_QAP_SIZE * MAX_QAP_SIZE];
  int n;
};

void read_instance(char *filename, struct QAP *qap);
int evaluate_solution(int *sol, struct QAP *qap);
int get_delta(int *sol, int i, int j, struct QAP *qap);

void heuristic(int *solution, struct QAP *qap);
int localsearchgreedy(int *solution, struct QAP *qap, int *evaluated,
                      int *steps);
int localsearchsteepest(int *solution, struct QAP *qap, int *evaluated,
                        int *steps);
int randomwalk(int *solution, struct QAP *qap, int *evaluated, int *steps);