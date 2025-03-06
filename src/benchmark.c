#include <unistd.h>
#include <time.h>
#include "random.h"

int alg() {
    // dummy algorithm that sleeps for a random duration of time
    usleep(randint(2000) * 100);
}

#define MINIMUM_ITERS 30
#define MINIMUM_TIME 3

int main() {
    int counter = 0;
    time_t start = time(NULL);

    do {
        counter += 1;
        printf("%d\n", counter);
        alg();
    } while (counter <= MINIMUM_ITERS || time(NULL) - start <= MINIMUM_TIME);
}
