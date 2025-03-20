#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_QAP_SIZE 256

int A[MAX_QAP_SIZE * MAX_QAP_SIZE];
int B[MAX_QAP_SIZE * MAX_QAP_SIZE];
int n;

void read_instance(char *filename);
int evaluate_solution(int *sol);
void heuristic(int *solution);