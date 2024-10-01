#!/bin/bash

export ISA_TEST_DIR=./test_files/share/riscv-tests/isa

if [ ! -d "$ISA_TEST_DIR" ]; then
    exit 1
fi

ENABLED_TESTS_FILE="./enabled_isa_tests.txt"
if [ ! -f "$ENABLED_TESTS_FILE" ]; then
    exit 1
fi

mapfile -t enabled_tests < "$ENABLED_TESTS_FILE"

OUTPUT_FILE="disabled_isa_tests.txt"
> "$OUTPUT_FILE"

find "$ISA_TEST_DIR" -type f ! -name 'Makefile' ! -name '*.dump' -printf "%f\n" | while read -r file; do
    if [[ ! " ${enabled_tests[@]} " =~ " ${file} " ]]; then
        echo "$file" >> "$OUTPUT_FILE"
    fi
done

exit 0
