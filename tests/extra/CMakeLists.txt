
include(AddGoogleTest)

add_executable(extra_unit_tests)
target_compile_features(extra_unit_tests PUBLIC cxx_std_20)

target_sources(
  extra_unit_tests
  PRIVATE
    synch.test.cpp
    enum_flags.test.cpp
)

target_link_libraries(
  extra_unit_tests
  PRIVATE
    extra::extra
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(extra_unit_tests)
