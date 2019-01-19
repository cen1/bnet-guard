add_executable(guard_test test.cpp)

set(GTEST_ROOT ./depends/gtest)

set_target_properties(guard_test PROPERTIES OUTPUT_NAME guard_test)

find_package(GTest REQUIRED)

include_directories(../guard ${GTEST_INCLUDE_DIRS})

target_link_libraries (guard_test PRIVATE gtest)

add_test(NAME guard_test COMMAND guard_test)
