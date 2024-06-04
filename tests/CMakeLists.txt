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
  set(ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${NUM_CORES})
endif()

message(STATUS "Found " ${NUM_CORES} " cores in machine (for ctest)")

add_custom_target (regress DEPENDS dromajo)

add_custom_command(TARGET regress POST_BUILD
                   COMMAND ctest --output-on-failure)

add_test(NAME stf_gen_test
         COMMAND ${CMAKE_COMMAND} -E env bash -c "bash run_bare_metal.sh 2>&1"
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/stf_gen)