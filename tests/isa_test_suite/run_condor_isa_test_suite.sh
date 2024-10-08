#!/bin/bash
export OPT='--stf_priv_modes USHM --stf_force_zero_sha'
export SIM_BIN=../../bin/cpm_dromajo
export RISCV_TEST_DIR=./condor-test-files/share/riscv-tests/isa

passed_tests=0
failed_tests=0
total_tests=0

echo "Using simulator binary: $SIM_BIN"
echo "Running all tests from directory: $RISCV_TEST_DIR"

if [ ! -d "$RISCV_TEST_DIR" ]; then
    echo "Error: Test directory $RISCV_TEST_DIR does not exist"
    exit 1
fi

# Find all test files in the directory, skipping .gitignore, Makefile, and .dump files
test_files=($(find "$RISCV_TEST_DIR" -type f ! -name '*.dump' ! -name '.gitignore' ! -name 'Makefile'))

total_tests_count=${#test_files[@]}

if [ "$total_tests_count" -eq 0 ]; then
    echo "Error: No valid test files found in $RISCV_TEST_DIR"
    exit 1
fi

run_test() {
    local test_file="$1"
    local current_test_number="$2"
    echo -n "Test $current_test_number/$total_tests_count: $(basename "$test_file") ... "

    # Capture the output of the simulator
    result=$($SIM_BIN $OPT "$test_file" 2>&1)
    EXIT_CODE=$?

    if [ $EXIT_CODE -eq 0 ]; then
        echo "PASSED"
        passed_tests=$((passed_tests + 1))
    else
        echo "FAILED ($(echo "$result" | head -n 1))"
        failed_tests=$((failed_tests + 1))
    fi
}

current_test_number=1
for test_file in "${test_files[@]}"; do
    run_test "$test_file" "$current_test_number"
    current_test_number=$((current_test_number + 1))
done

echo
echo "Total tests run: $total_tests_count"
echo "Tests passed: $passed_tests"
echo "Tests failed: $failed_tests"

exit $failed_tests
