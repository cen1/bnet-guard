add_executable(guard_test test.cpp)

set(GTEST_ROOT ./depends/gtest)

set_target_properties(guard_test PROPERTIES OUTPUT_NAME guard_test)

find_package(GTest REQUIRED)

include_directories(../guard ${GTEST_INCLUDE_DIRS})

if (NOT ${GTEST_LIBRARY_DEBUG})
  message(WARNING "gtest debug libraries not found, using release instead")
  set(GTEST_LIBRARY_DEBUG ${GTEST_LIBRARIES})
endif()

target_link_libraries (guard_test debug ${GTEST_LIBRARY_DEBUG} optimized ${GTEST_LIBRARIES} )

add_test(NAME guard_test COMMAND guard_test)
