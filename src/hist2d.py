import numpy as np
import pandas as pd

BOX = 10  # Define grid size
CSV_FILE = "data.csv"  # Input file

# Load data
df = pd.read_csv(CSV_FILE, names=["x", "y", "z"])

# Convert (x, y) to bin indices
bin_x = (df["x"] / BOX * BOX).astype(int)
bin_y = (df["y"] / BOX * BOX).astype(int)

# Create histogram
histogram = np.zeros((BOX, BOX), dtype=int)

# Populate histogram
for bx, by in zip(bin_x, bin_y):
    if 0 <= bx < BOX and 0 <= by < BOX:
        histogram[bx, by] += 1

# Print histogram
print(histogram)
