#include "random.h"

#define ARR_SIZE 30

int main() {
    int i;
    int array[ARR_SIZE];

    printf("Generating random permutation of %d elements: \n", ARR_SIZE);
    random_permutation(array, ARR_SIZE);
    for(i = 0; i < ARR_SIZE; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}
