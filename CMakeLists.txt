cmake_minimum_required(VERSION 3.28)

project(
  extra
  DESCRIPTION "General-purpose utility components"
  LANGUAGES CXX
  VERSION 0.1.1
)

option(
  EXTRA_ENABLE_TESTS
  "Enable building tests. Default: ${PROJECT_IS_TOP_LEVEL}."
  ${PROJECT_IS_TOP_LEVEL}
)

option(EXTRA_ENABLE_INSTALL "Enable installation rules. Default: ON."  ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(PreventInSouceBuilds)
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

if(EXTRA_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(extern/googletest)
  add_subdirectory(tests/extra)
endif(EXTRA_ENABLE_TESTS)

if(EXTRA_ENABLE_INSTALL)
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
endif(EXTRA_ENABLE_INSTALL)
