cmake_minimum_required(VERSION 3.10)

project(dromajo_tests)

message(STATUS "Configuring dromajo regress target")

include_directories(${PROJECT_SOURCE_DIR}/..)

# Enable CTset
include(CTest)

# Handy parallel processor library
include(ProcessorCount)
ProcessorCount(NUM_CORES)

if (NOT NUM_CORES EQUAL 0)
  set(CTEST_BUILD_FLAGS -j${NUM_CORES})
  set(ctest_test_args "--parallel ${NUM_CORES}")
endif()

message(STATUS "Found " ${NUM_CORES} " cores in machine (for ctest)")

add_subdirectory(stf_load_store)

add_custom_target (regress)

add_dependencies(regress dromajo stf_load_store_bare stf_load_store_linux)

add_custom_command(TARGET regress POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
                   COMMAND ${CMAKE_CTEST_COMMAND} ${ctest_test_args})

add_test(NAME stf_load_store_baremetal
         COMMAND ${CMAKE_COMMAND} -E env bash -c "bash scripts/run_baremetal_test.sh ${CMAKE_BINARY_DIR}/bin/stf_load_store.bare.riscv 2>&1"
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/stf_load_store)
         
add_test(NAME stf_load_store_linux
         COMMAND ${CMAKE_COMMAND} -E env bash -c "bash scripts/run_linux_test.sh ${CMAKE_BINARY_DIR}/bin/stf_load_store.linux.riscv 2>&1"
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/stf_load_store)

add_test(NAME stf_gen_ad_hoc
         COMMAND ${CMAKE_COMMAND} -E env bash -c "bash run_bare_metal.sh 2>&1"
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/stf_gen)