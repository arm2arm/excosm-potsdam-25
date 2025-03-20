#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BOX 100.0
#define NBINS 100

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <csvfile>\n", argv[0]);
        return 1;
    }
    
    // Initialize histogram array
    int histogram[NBINS][NBINS] = {0};
    
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
    
    // Start timing
    clock_t start = clock();
    
    // Process data
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%lf,%lf,%lf", &x, &y, &z) == 3) {
            // Calculate bin indices
            int i = (int)(x * NBINS / BOX);
            int j = (int)(y * NBINS / BOX);
            
            // Bounds check
            if (i >= 0 && i < NBINS && j >= 0 && j < NBINS) {
                histogram[i][j]++;
            }
        }
    }
    
    // End timing
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    fclose(fp);
    
    // Output histogram to file
    FILE *out = fopen("histogram_serial.dat", "w");
    if (!out) {
        perror("Failed to open output file");
        return 1;
    }
    
    for (int i = 0; i < NBINS; i++) {
        for (int j = 0; j < NBINS; j++) {
            fprintf(out, "%d ", histogram[i][j]);
        }
        fprintf(out, "\n");
    }
    fclose(out);
    
    printf("Serial histogramming complete in %.4f seconds\n", cpu_time_used);
    
    return 0;
}
