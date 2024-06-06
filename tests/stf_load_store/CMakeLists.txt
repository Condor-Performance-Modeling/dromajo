cmake_minimum_required(VERSION 3.10)

# Set the project name and language
project(bmi_median C)

# Specify the compiler
set(CMAKE_C_COMPILER "/data/tools/riscv64-unknown-elf/bin/riscv64-unknown-elf-gcc")

# Set the compile flags
set(CMAKE_C_FLAGS "-march=rv64gc -mabi=lp64d -nostdlib -nostartfiles -mcmodel=medany -O3 -ffast-math -funsafe-math-optimizations -finline-functions -fno-common -fno-builtin-printf -flto -fno-tree-loop-distribute-patterns -static -std=gnu99")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPREALLOCATE=1 -D__riscv=1")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/common)

# Linker script
set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/common/linker/common_proposed.ld")
set(CMAKE_EXE_LINKER_FLAGS "-T${LINKER_SCRIPT} -lm -lgcc")

# Source files
file(GLOB COMMON_SOURCES "${PROJECT_SOURCE_DIR}/common/*.c" "${PROJECT_SOURCE_DIR}/common/*.S")
file(GLOB MEDIAN_SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

# Add the executable
add_executable(bmi_median.bare.riscv ${COMMON_SOURCES} ${MEDIAN_SOURCES})