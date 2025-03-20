#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define BOX 100.0
#define NBINS 100

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <csvfile>\n", argv[0]);
        return 1;
    }
    
    // Initialize histogram array
    int histogram[NBINS][NBINS] = {0};
    
    // Read all data into memory first for better parallelization
    double *x_data = NULL, *y_data = NULL, *z_data = NULL;
    int data_size = 0, data_capacity = 1000;
    
    x_data = malloc(data_capacity * sizeof(double));
    y_data = malloc(data_capacity * sizeof(double));
    z_data = malloc(data_capacity * sizeof(double));
    
    if (!x_data || !y_data || !z_data) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Open file
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }
    
    char line[1024];
    double x, y, z;
    
    // Skip header if it exists
    fgets(line, sizeof(line), fp);
    
    // Read all data into memory
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%lf,%lf,%lf", &x, &y, &z) == 3) {
            if (data_size >= data_capacity) {
                data_capacity *= 2;
                x_data = realloc(x_data, data_capacity * sizeof(double));
                y_data = realloc(y_data, data_capacity * sizeof(double));
                z_data = realloc(z_data, data_capacity * sizeof(double));
                
                if (!x_data || !y_data || !z_data) {
                    printf("Memory reallocation failed\n");
                    return 1;
                }
            }
            x_data[data_size] = x;
            y_data[data_size] = y;
            z_data[data_size] = z;
            data_size++;
        }
    }
    fclose(fp);
    
    printf("Read %d data points from file\n", data_size);
    
    // Method 1: Using atomic updates
    printf("\n--- Method 1: Using atomic updates ---\n");
    memset(histogram, 0, sizeof(histogram)); // Reset histogram
    
    double start_time = omp_get_wtime();
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int idx = 0; idx < data_size; idx++) {
            // Calculate bin indices
            int i = (int)(x_data[idx] * NBINS / BOX);
            int j = (int)(y_data[idx] * NBINS / BOX);
            
            // Bounds check
            if (i >= 0 && i < NBINS && j >= 0 && j < NBINS) {
                #pragma omp atomic
                histogram[i][j]++;
            }
        }
    }
    
    double end_time = omp_get_wtime();
    printf("OpenMP (atomic) completed in %.4f seconds with %d threads\n", 
           end_time - start_time, omp_get_max_threads());
    
    // Output histogram
    FILE *out1 = fopen("histogram_openmp_atomic.dat", "w");
    for (int i = 0; i < NBINS; i++) {
        for (int j = 0; j < NBINS; j++) {
            fprintf(out1, "%d ", histogram[i][j]);
        }
        fprintf(out1, "\n");
    }
    fclose(out1);
    
    // Method 2: Using private histograms
    printf("\n--- Method 2: Using private histograms ---\n");
    memset(histogram, 0, sizeof(histogram)); // Reset histogram
    
    start_time = omp_get_wtime();
    
    #pragma omp parallel
    {
        // Create private histogram for each thread
        int private_hist[NBINS][NBINS] = {0};
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int idx = 0; idx < data_size; idx++) {
            // Calculate bin indices
            int i = (int)(x_data[idx] * NBINS / BOX);
            int j = (int)(y_data[idx] * NBINS / BOX);
            
            // Bounds check
            if (i >= 0 && i < NBINS && j >= 0 && j < NBINS) {
                private_hist[i][j]++;
            }
        }
        
        // Merge private histograms into the global one
        #pragma omp critical
        {
            for (int i = 0; i < NBINS; i++) {
                for (int j = 0; j < NBINS; j++) {
                    histogram[i][j] += private_hist[i][j];
                }
            }
        }
    }
    
    end_time = omp_get_wtime();
    printf("OpenMP (private) completed in %.4f seconds with %d threads\n", 
           end_time - start_time, omp_get_max_threads());
    
    // Output histogram
    FILE *out2 = fopen("histogram_openmp_private.dat", "w");
    for (int i = 0; i < NBINS; i++) {
        for (int j = 0; j < NBINS; j++) {
            fprintf(out2, "%d ", histogram[i][j]);
        }
        fprintf(out2, "\n");
    }
    fclose(out2);
    
    // Clean up
    free(x_data);
    free(y_data);
    free(z_data);
    
    return 0;
}
