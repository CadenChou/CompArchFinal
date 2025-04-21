#!/bin/bash

# Check if a file path is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <file-path>"
    exit 1
fi

# Get the file path from the argument
FILE="$1"

# Check if the file exists
if [ ! -f "$FILE" ]; then
    echo "Error: File not found!"
    exit 1
fi


# Extract relevant values from the stats file
simInsts=$(grep -E '^simInsts' "$FILE" | awk '{print $2}')
simTicks=$(grep -E '^simTicks' "$FILE" | awk '{print $2}')

# Identify the pattern that corresponds to "supersecretdata"
if [[ "$simTicks" -lt "7596055000" &&  "$simTicks" -gt "596055000" ]]; then
    echo "supersecretdata"
else
    echo "data is not secret"
fi