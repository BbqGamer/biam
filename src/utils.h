#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int mod(int a, int b);
int randint(int n);

void random_permutation(int *array, int n);
void random_pair(int *a, int *b, int n);

int argmin(int *array, int n);
int argmax(int *array, int n);

void swap(int *solution, int i, int j);
