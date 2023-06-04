#!/bin/bash

# find .h and .cpp files in the current directory and its subdirectories
find . -name "*.h" -o -name "*.cpp" | while read file; do
    # format each file with clang-format
    clang-format -i "$file"
done

# add the formatted files to the git repository and commit them
git add .
git commit -m "Format"
