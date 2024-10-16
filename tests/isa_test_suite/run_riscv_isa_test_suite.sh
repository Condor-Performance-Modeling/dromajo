#!/bin/bash
export OPT='--ctrlc --stf_priv_modes USHM --stf_force_zero_sha'
export SIM_BIN=../../bin/cpm_dromajo
export RISCV_TEST_DIR=./riscv-test-files/share/riscv-tests/isa
ALLOWED_TESTS_FILE="isa_tests_list.txt"

passed_tests=0
failed_tests=0
total_tests=0
test_counter=0
last_test_duration=0

passed_tests_list=()

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

total_start_time=$(date +%s)

trap ctrl_c_handler SIGINT
ctrl_c_handler() {
    echo "Stopping test execution..."
    exit 1
}

run_test() {
    local test_file="$1"
    test_counter=$((test_counter + 1))
    echo "Running test #$test_counter: $test_file"
    start_test_time=$(date +%s)

    $SIM_BIN $OPT "$test_file" &
    SIM_PID=$!

    wait $SIM_PID
    EXIT_CODE=$?

    end_test_time=$(date +%s)
    test_time=$((end_test_time - start_test_time))
    last_test_duration=$test_time

    if [ $EXIT_CODE -eq 0 ]; then
        echo "Test $test_file passed"
        passed_tests=$((passed_tests + 1))
        passed_tests_list+=("$test_file")
    else
        echo "Test $test_file failed with exit code $EXIT_CODE"
        failed_tests=$((failed_tests + 1))
    fi

    echo "Test duration: $test_time seconds"
    echo "---------------------------"
}

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
        run_test "$full_test_path"
    else
        echo "Warning: Test file $full_test_path does not exist"
        echo "Test $test_file failed due to missing file"
        failed_tests=$((failed_tests + 1))
    fi
done

total_end_time=$(date +%s)
total_elapsed_time=$((total_end_time - total_start_time))

echo
echo "Summary:"
echo "Total tests run: $total_tests_count"
echo "Tests passed: $passed_tests"
echo "Tests failed: $failed_tests"
echo "Last test duration: $last_test_duration seconds"
echo "Total execution time: $total_elapsed_time seconds"

exit $failed_tests