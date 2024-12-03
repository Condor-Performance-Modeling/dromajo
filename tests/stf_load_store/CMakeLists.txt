cmake_minimum_required(VERSION 3.10)

project(stf_load_store)

set(COMMON_TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../common_test_files)

file(GLOB SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
file(GLOB LINKER_DIR ${COMMON_TEST_FILES}/linker)

# Custom target to run the bash script to check RISCV for baremetal
add_custom_target(check_stf_load_store_bare_dependencies
    COMMAND bash scripts/check_riscv_env_variable.sh
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Custom target to run the bash script to check RISCV_LINUX for Linux
add_custom_target(check_stf_load_store_linux_dependencies
    COMMAND bash scripts/check_riscv_linux_env_variable.sh
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# BAREMETAL:
# Custom command to run baremetal build using RISCV environment variable directly
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bin/stf_load_store.bare.riscv
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND $ENV{RISCV}/bin/riscv64-unknown-elf-gcc
    -march=rv64gc_zfhmin
    -mabi=lp64d
    -nostartfiles -mcmodel=medany  
    -fno-tree-loop-distribute-patterns  
    -I${COMMON_DIR}
    -T${LINKER_DIR}/common_proposed.ld ${COMMON_DIR}/syscalls.c ${COMMON_DIR}/crt.S 
    ${SOURCE_DIR}/*.c
    -DBAREMETAL_BUILD
    -o ${CMAKE_BINARY_DIR}/bin/stf_load_store.bare.riscv
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${COMMON_DIR}/syscalls.c ${COMMON_DIR}/crt.S ${SOURCE_DIR}/*.c $ENV{RISCV}/bin/riscv64-unknown-elf-gcc
)

add_custom_target(stf_load_store_bare DEPENDS check_stf_load_store_bare_dependencies ${CMAKE_BINARY_DIR}/bin/stf_load_store.bare.riscv)
set_target_properties(stf_load_store_bare PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_custom_command(TARGET stf_load_store_bare POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E echo "Copying bare metal elf..."
                   COMMAND cp ${CMAKE_BINARY_DIR}/bin/stf_load_store.bare.riscv ${PROJECT_SOURCE_DIR}/common)

# LINUX:
# Custom command to run Linux build using RISCV_LINUX environment variable directly
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bin/stf_load_store.linux.riscv
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND $ENV{RISCV_LINUX}/bin/riscv64-unknown-linux-gnu-gcc
    -march=rv64gc -mabi=lp64d -mcmodel=medany
    -O3
    -ffast-math -funsafe-math-optimizations -finline-functions 
    -fno-common -fno-builtin-printf -flto -fno-tree-loop-distribute-patterns    
    -I${COMMON_DIR}
    -static -std=gnu99 -D__linux=1
    ${COMMON_DIR}/lnx_calls.c 
    ${SOURCE_DIR}/*.c 
    -o ${CMAKE_BINARY_DIR}/bin/stf_load_store.linux.riscv
    -lm -lgcc 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${COMMON_DIR}/lnx_calls.c ${SOURCE_DIR}/*.c $ENV{RISCV_LINUX}/bin/riscv64-unknown-linux-gnu-gcc
)

add_custom_target(stf_load_store_linux DEPENDS check_stf_load_store_linux_dependencies ${CMAKE_BINARY_DIR}/bin/stf_load_store.linux.riscv)
set_target_properties(stf_load_store_linux PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_custom_command(TARGET stf_load_store_linux POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E echo "Copying linux elf..."
                   COMMAND cp ${CMAKE_BINARY_DIR}/bin/stf_load_store.linux.riscv ${PROJECT_SOURCE_DIR}/common
                   COMMAND ${CMAKE_COMMAND} -E env bash -c "bash scripts/create_linux_collateral.sh 2>&1"
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})