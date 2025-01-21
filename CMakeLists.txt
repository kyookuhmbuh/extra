cmake_minimum_required(VERSION 3.28)

project(
  extra
  DESCRIPTION "General-purpose utility components"
  LANGUAGES CXX
  VERSION 0.1.1
)

option(
  EXTRA_BUILD_TESTS
  "Enable building tests. Default: ${PROJECT_IS_TOP_LEVEL}."
  ${PROJECT_IS_TOP_LEVEL}
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilationDatabase)

add_library(extra INTERFACE)
add_library(extra::extra ALIAS extra)

target_compile_features(extra INTERFACE cxx_std_20)

target_sources(
  extra
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(extra PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)
add_subdirectory(include/extra)

if(EXTRA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests/extra)
endif()

include(GNUInstallDirs)

install(
  TARGETS extra
  EXPORT ${PROJECT_NAME}_targets
  FILE_SET HEADERS
)

install(
  EXPORT ${PROJECT_NAME}_targets
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CPack)
