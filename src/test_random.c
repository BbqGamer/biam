#include "random.h"

#define REPS 10
#define ARR_SIZE 30

int main() {
    int i, j;
    int array[ARR_SIZE];

    printf("Generating random permutation of %d elements: \n", ARR_SIZE);
    for(i = 0; i < REPS; i++) {
        random_permutation(array, ARR_SIZE);
        for(j = 0; j < ARR_SIZE; j++) {
            printf("%d ", array[j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Generating random pairs:");
    for(i = 0; i < 10; i++) {
        int a, b;
        random_pair(&a, &b, ARR_SIZE);
        printf("%d %d\n", a, b);

    }
}
