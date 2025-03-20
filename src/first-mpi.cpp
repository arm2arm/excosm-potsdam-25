#include <mpi.h>
#include <iostream>
#include <vector>
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 1000000;
    double local_sum = 0.0;
    for (int i = rank; i < N; i += size)
        local_sum += 1.0 / (i + 1);

    double global_sum = 0.0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
        std::cout << "Total Sum: " << global_sum << std::endl;
    MPI_Finalize();
    return 0;
}
