#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

#define BOX 100.0
#define NBINS 100

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time, end_time;
    
    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Check command line arguments
    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <csvfile>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    // Initialize local and global histogram arrays
    int local_histogram[NBINS][NBINS] = {0};
    int global_histogram[NBINS][NBINS] = {0};
    
    // Only root process reads the file
    if (rank == 0) {
        printf("Starting parallel histogram with %d processes\n", size);
        
        // Open file
        FILE *fp = fopen(argv[1], "r");
        if (!fp) {
            perror("Failed to open file");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        
        // Count total number of data points
        char line[1024];
        long total_lines = 0;
        
        // Skip header if it exists
        fgets(line, sizeof(line), fp);
        
        while (fgets(line, sizeof(line), fp)) {
            total_lines++;
        }
        
        // Reset file position
        rewind(fp);
        fgets(line, sizeof(line), fp); // Skip header again
        
        printf("Found %ld data points to process\n", total_lines);
        
        // Allocate memory for data
        double (*data)[3] = malloc(total_lines * sizeof(*data));
        if (!data) {
            perror("Failed to allocate memory");
            fclose(fp);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        
        // Read all data points
        long i = 0;
        while (i < total_lines && fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "%lf,%lf,%lf", &data[i][0], &data[i][1], &data[i][2]) == 3) {
                i++;
            }
        }
        fclose(fp);
        
        // Calculate send counts and displacements for scatter
        int *sendcounts = malloc(size * sizeof(int));
        int *displs = malloc(size * sizeof(int));
        
        int base_count = total_lines / size;
        int remainder = total_lines % size;
        
        int offset = 0;
        for (i = 0; i < size; i++) {
            sendcounts[i] = base_count;
            if (i < remainder) {
                sendcounts[i]++;
            }
            sendcounts[i] *= 3; // Each data point has 3 doubles
            displs[i] = offset;
            offset += sendcounts[i];
        }
        
        // Start timing
        start_time = MPI_Wtime();
        
        // Send data counts to all processes
        int local_count;
        MPI_Scatter(sendcounts, 1, MPI_INT, &local_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        // Allocate receive buffer on all processes
        double *local_data = malloc(local_count * sizeof(double));
        
        // Scatter data to all processes
        MPI_Scatterv(data, sendcounts, displs, MPI_DOUBLE, 
                     local_data, local_count, MPI_DOUBLE, 
                     0, MPI_COMM_WORLD);
        
        // Free memory
        free(data);
        free(sendcounts);
        free(displs);
        
        // Process root's portion of data
        for (i = 0; i < local_count / 3; i++) {
            double x = local_data[i*3];
            double y = local_data[i*3+1];
            
            // Calculate bin indices
            int bin_i = (int)(x * NBINS / BOX);
            int bin_j = (int)(y * NBINS / BOX);
            
            // Bounds check
            if (bin_i >= 0 && bin_i < NBINS && bin_j >= 0 && bin_j < NBINS) {
                local_histogram[bin_i][bin_j]++;
            }
        }
        
        free(local_data);
    } else {
        // Non-root processes
        // Receive count from root
        int local_count;
        MPI_Scatter(NULL, 0, MPI_INT, &local_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        // Allocate receive buffer
        double *local_data = malloc(local_count * sizeof(double));
        
        // Receive data from root
        MPI_Scatterv(NULL, NULL, NULL, MPI_DOUBLE, 
                     local_data, local_count, MPI_DOUBLE, 
                     0, MPI_COMM_WORLD);
        
        // Process local portion of data
        for (int i = 0; i < local_count / 3; i++) {
            double x = local_data[i*3];
            double y = local_data[i*3+1];
            
            // Calculate bin indices
            int bin_i = (int)(x * NBINS / BOX);
            int bin_j = (int)(y * NBINS / BOX);
            
            // Bounds check
            if (bin_i >= 0 && bin_i < NBINS && bin_j >= 0 && bin_j < NBINS) {
                local_histogram[bin_i][bin_j]++;
            }
        }
        
        free(local_data);
    }
    
    // Reduce all local histograms to the global one
    MPI_Reduce(&local_histogram[0][0], &global_histogram[0][0], NBINS*NBINS, 
               MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Only root writes the results
    if (rank == 0) {
        // End timing
        end_time = MPI_Wtime();
        
        // Output histogram to file
        FILE *out = fopen("histogram_parallel.dat", "w");
        if (!out) {
            perror("Failed to open output file");
            MPI_Finalize();
            return 1;
        }
        
        for (int i = 0; i < NBINS; i++) {
            for (int j = 0; j < NBINS; j++) {
                fprintf(out, "%d ", global_histogram[i][j]);
            }
            fprintf(out, "\n");
        }
        fclose(out);
        
        printf("Parallel histogramming complete in %.4f seconds with %d processes\n", 
               end_time - start_time, size);
    }
    
    MPI_Finalize();
    return 0;
}