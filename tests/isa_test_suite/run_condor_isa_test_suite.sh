#!/bin/bash
export OPT='--stf_priv_modes USHM --stf_force_zero_sha'
export SIM_BIN=../../bin/cpm_dromajo
export RISCV_TEST_DIR=./condor-test-files/share/riscv-tests/isa
ALLOWED_TESTS_FILE="condor_tests_list.txt"

passed_tests=0
failed_tests=0
total_tests=0
test_counter=0

echo "Using simulator binary: $SIM_BIN"
echo "Running tests from directory: $RISCV_TEST_DIR"

if [ ! -d "$RISCV_TEST_DIR" ]; then
    echo "Error: Test directory $RISCV_TEST_DIR does not exist"
    exit 1
fi

if [ ! -f "$ALLOWED_TESTS_FILE" ]; then
    echo "Error: Allowed tests file $ALLOWED_TESTS_FILE does not exist"
    exit 1
fi

mapfile -t allowed_test_files < "$ALLOWED_TESTS_FILE"

trap ctrl_c_handler SIGINT
ctrl_c_handler() {
    echo "Stopping test execution..."
    exit 1
}

run_test() {
    local test_file="$1"
    local current_test_number="$2"
    echo -n "Test $current_test_number/$total_tests_count: condor isa test - $(basename "$test_file") ... "

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
for test_file in "${allowed_test_files[@]}"; do

    test_file=$(echo "$test_file" | xargs)
    
    if [[ -z "$test_file" ]]; then
        continue
    fi

    if [[ "$test_file" =~ ^# ]] || [[ "$test_file" =~ ^x ]]; then
        continue
    fi

    full_test_path="$RISCV_TEST_DIR/$test_file"
    total_tests_count=$((total_tests_count + 1))

    if [ -f "$full_test_path" ]; then
        run_test "$full_test_path" "$current_test_number"
        current_test_number=$((current_test_number + 1))
    else
        echo "Warning: Test file $full_test_path does not exist"
        echo "Test $test_file failed due to missing file"
        failed_tests=$((failed_tests + 1))
    fi
done

echo
echo "Total tests run: $total_tests_count"
echo "Tests passed: $passed_tests"
echo "Tests failed: $failed_tests"

exit $failed_tests
