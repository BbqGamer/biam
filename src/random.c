#include "random.h"

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

void random_permutation(int *array, int n) {
    int i;
    for (i = 0; i < n; i++) {
        array[i] = i;
    }

    for (i--; i >= 0; i--) {
        int j = mod(rand(), i + 1);
        int tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

