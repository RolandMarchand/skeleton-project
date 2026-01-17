enable_testing()

add_executable(test_dummy EXCLUDE_FROM_ALL
  test_dummy.c
)
target_link_libraries(test_dummy PRIVATE unity)
target_include_directories(test_dummy PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME TestDummy COMMAND test_dummy)

add_custom_target(tests
  DEPENDS test_dummy
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
  COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
)
