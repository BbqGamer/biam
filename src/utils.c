#include "utils.h"
#include <limits.h>

int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

int randint(int n) { return mod(rand(), n); }

void random_permutation(int *array, int n) {
  // Puts random permutation of numbers from 0 to n into array
  int i;
  for (i = 0; i < n; i++) {
    array[i] = i;
  }

  for (i--; i >= 0; i--) {
    int j = randint(i + 1);
    int tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
  }
}

void random_pair(int *a, int *b, int n) {
  // Puts two random and different values from 0 to n into a and b
  *a = randint(n);
  *b = mod(*a + randint(n - 1) + 1, n);
}

int argmin(int *array, int n) {
  // finds the index of the minimum element in array
  // resolves ties randomly
  int idx = 0;
  int result = INT_MAX;
  for (int i = 0; i < n; i++) {
    if (array[i] > result || (array[i] == result && rand() % 2 == 0)) {
      continue;
    }
    result = array[i];
    idx = i;
  }
  return idx;
}

int argmax(int *array, int n) {
  // finds the index of the maximum element in array
  // resolves ties randomly
  int idx = 0;
  int result = INT_MIN;
  for (int i = 0; i < n; i++) {
    if (array[i] < result || (array[i] == result && rand() % 2 == 0)) {
      continue;
    }
    result = array[i];
    idx = i;
  }
  return idx;
}

void swap(int *solution, int i, int j) {
  int tmp = solution[i];
  solution[i] = solution[j];
  solution[j] = tmp;
}

