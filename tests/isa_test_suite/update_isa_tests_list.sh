#!/bin/bash

export ISA_TEST_DIR=./riscv-test-files/share/riscv-tests/isa
ISA_TEST_LIST_FILE="isa_tests_list.txt"

if [ ! -d "$ISA_TEST_DIR" ]; then
    echo "Error: Test directory $ISA_TEST_DIR does not exist"
    exit 1
fi

if [ ! -f "$ISA_TEST_LIST_FILE" ]; then
    echo "Error: ISA test list file $ISA_TEST_LIST_FILE does not exist"
    exit 1
fi

mapfile -t isa_test_list < "$ISA_TEST_LIST_FILE"

enabled_count=0
disabled_count=0
new_disabled_count=0

test_exists_in_list() {
    local test_name="$1"
    for test in "${isa_test_list[@]}"; do
        stripped_test=$(echo "$test" | sed 's/^x //')
        if [[ "$stripped_test" == "$test_name" ]]; then
            if [[ "$test" =~ ^x ]]; then
                return 1
            else
                return 2
            fi
        fi
    done
    return 0
}

total_tests=$(find "$ISA_TEST_DIR" -type f ! -name '.gitignore' ! -name 'Makefile' ! -name '*.dump' -printf "%f\n" | wc -l)
current_test_number=0

# Iterate through files in the ISA test directory using a standard for loop
for file in $(find "$ISA_TEST_DIR" -type f ! -name '.gitignore' ! -name 'Makefile' ! -name '*.dump' -printf "%f\n"); do
    current_test_number=$((current_test_number + 1))
    echo "Checking file $current_test_number/$total_tests: $file"
    
    test_exists_in_list "$file"
    case $? in
        1) 
            echo "$file exists in isa_tests_list.txt (disabled)"
            disabled_count=$((disabled_count + 1))
            ;;
        2) 
            echo "$file exists in isa_tests_list.txt (enabled)"
            enabled_count=$((enabled_count + 1))
            ;;
        0) 
            echo "Adding new disabled test: $file"
            echo "x $file" >> "$ISA_TEST_LIST_FILE"
            new_disabled_count=$((new_disabled_count + 1))
            ;;
    esac
done

echo "Summary:"
echo "Enabled tests found: $enabled_count"
echo "Disabled tests found: $disabled_count"
echo "New disabled tests added: $new_disabled_count"

exit 0
