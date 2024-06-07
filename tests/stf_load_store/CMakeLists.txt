cmake_minimum_required(VERSION 3.10)

project(stf_load_store)

set(COMPILER "/data/tools/riscv64-unknown-elf/bin/riscv64-unknown-elf-gcc")

# Gather source files
file(GLOB SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
file(GLOB LINKER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/linker)

# Debug messages to confirm paths
message(STATUS "Source files: ${SOURCE_DIR}")
message(STATUS "Common files: ${COMMON_DIR}")
message(STATUS "Linker script: ${LINKER_DIR}")

# Custom command to run the known working command
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bin/stf_load_store.bare.riscv
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${COMPILER}
    -nostartfiles -mcmodel=medany  
    -O3 
    -fno-builtin-printf -flto -fno-tree-loop-distribute-patterns  
    -I${COMMON_DIR}
    -T${LINKER_DIR}/common_proposed.ld ${COMMON_DIR}/syscalls.c ${COMMON_DIR}/crt.S 
    ${SOURCE_DIR}/*.c 
    -o ${CMAKE_BINARY_DIR}/bin/stf_load_store.bare.riscv
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${COMMON_DIR}/syscalls.c ${COMMON_DIR}/crt.S ${SOURCE_DIR}/*.c ${COMPILER}
)

# Add a custom target to build the project
add_custom_target(stf_load_store DEPENDS ${CMAKE_BINARY_DIR}/bin/stf_load_store.bare.riscv)

# Specify the output directory for the binary
set_target_properties(stf_load_store PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")