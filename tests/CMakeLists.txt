project(dromajo_tests)

message(STATUS "Configuring dromajo regress target")

# Enable CTset
include(CTest)

# Handy parallel processor library
include(ProcessorCount)
ProcessorCount(NUM_CORES)

if (NOT NUM_CORES EQUAL 0)
  set(CTEST_BUILD_FLAGS -j${NUM_CORES})
  set(ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${NUM_CORES})
endif()

message(STATUS "Found " ${NUM_CORES} " cores in machine (for ctest)")

add_custom_target (regress DEPENDS dromajo)

add_custom_command(TARGET regress POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E echo "Running run_bare_metal.sh script for regress target..."
                   COMMAND ${CMAKE_COMMAND} -E env bash ${PROJECT_SOURCE_DIR}/stf_gen/run_bare_metal.sh)

include_directories(${PROJECT_SOURCE_DIR}/..)