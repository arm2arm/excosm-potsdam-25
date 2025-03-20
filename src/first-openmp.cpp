#include <iostream>
#include <omp.h>

int main() {
    const int N = 1000000;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += 1.0 / (i + 1);
    }

    std::cout << "Sum: " << sum << std::endl;
    return 0;
}