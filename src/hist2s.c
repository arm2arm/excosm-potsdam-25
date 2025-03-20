#include <stdio.h>
#include <stdlib.h>

#define BOX 10  // Define the grid size
#define CSV_FILE "data.csv"  // Input file

int main() {
    int histogram[BOX][BOX] = {0};  // Initialize histogram

    FILE *file = fopen(CSV_FILE, "r");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    double x, y, z;
    while (fscanf(file, "%lf,%lf,%lf", &x, &y, &z) == 3) {
        int bin_x = (int)(x / BOX * BOX);
        int bin_y = (int)(y / BOX * BOX);

        if (bin_x >= 0 && bin_x < BOX && bin_y >= 0 && bin_y < BOX) {
            histogram[bin_x][bin_y]++;
        }
    }

    fclose(file);

    // Print histogram
    for (int i = 0; i < BOX; i++) {
        for (int j = 0; j < BOX; j++) {
            printf("%d ", histogram[i][j]);
        }
        printf("\n");
    }

    return 0;
}
