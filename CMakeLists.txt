cmake_minimum_required(VERSION 3.28)

project(extra
  VERSION 1.0.0
  LANGUAGES CXX)

add_library(extra STATIC)
add_library(extra::extra ALIAS extra)

target_compile_features(extra PUBLIC cxx_std_20)

target_sources(extra
  PUBLIC FILE_SET HEADERS
  BASE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
  FILES
    include/extra/args.hpp
    include/extra/atomic_flags.hpp
    include/extra/debug.hpp
    include/extra/enum_flags.hpp
    include/extra/extra.hpp
    include/extra/hash_combine.hpp
    include/extra/overloaded.hpp
    include/extra/synch.hpp
    include/extra/variant_alternative.hpp
    include/extra/variant_visitor.hpp
)

target_sources(extra
  PRIVATE
    src/lib.cpp
)

include(GNUInstallDirs)

install(TARGETS extra
  EXPORT ${PROJECT_NAME}_targets
  FILE_SET HEADERS
)

install(EXPORT ${PROJECT_NAME}_targets
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CPack)
