
# Guard to ensure this file is executed only for the top-level project.
if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

#[[
  Enables generation of `compile_commands.json`, which contains the exact
  compiler invocations for all translation units in a machine-readable format.

  When using the Ninja Multi-Config generator, CMake emits compilation commands
  for all configurations for each source file (e.g. Debug, Release,
  RelWithDebInfo, MinSizeRel).

  Observations from clangd logs show that clangd always selects commands
  corresponding to the Debug configuration, likely because it is the first
  configuration encountered in the file.
#]]
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#[[
  Creates a symbolic link to the generated `compile_commands.json` file.

  By default, clangd searches for `compile_commands.json` in the parent
  directories of the source file, so placing a link in the source root
  improves discoverability.
#]]
option(
  CMAKE_CREATE_COMPILE_COMMANDS_LINK
  "Default: ${CMAKE_EXPORT_COMPILE_COMMANDS}."
  ${CMAKE_EXPORT_COMPILE_COMMANDS}
)

#[[
  Creates a symbolic link to the build directory.

  This can be useful for tools that expect the compilation database to be
  located relative to a fixed directory (e.g. clangd with custom search paths).
#]]
option(
  CMAKE_CREATE_COMPILE_COMMANDS_DIR_LINK
  "Default: ${CMAKE_EXPORT_COMPILE_COMMANDS}."
  ${CMAKE_EXPORT_COMPILE_COMMANDS}
)

if(CMAKE_CREATE_COMPILE_COMMANDS_LINK)
  set(_ORIGINAL "${CMAKE_BINARY_DIR}/compile_commands.json")
  set(_LINKNAME "${CMAKE_SOURCE_DIR}/compile_commands.json")

  file(
    CREATE_LINK ${_ORIGINAL} ${_LINKNAME}
    RESULT _RESULT
    SYMBOLIC
  )

  if(_RESULT EQUAL 0)
    message(STATUS "Link created: ${_ORIGINAL}")
  else()
    message(WARNING "Link not created (${_RESULT})")
  endif()
endif()

if(CMAKE_CREATE_COMPILE_COMMANDS_DIR_LINK)
  set(_ORIGINAL "${CMAKE_BINARY_DIR}")
  set(_LINKNAME "${CMAKE_SOURCE_DIR}/out/.build")

  file(
    CREATE_LINK ${_ORIGINAL} ${_LINKNAME}
    RESULT _RESULT
    SYMBOLIC
  )

  if(_RESULT EQUAL 0)
    message(STATUS "Link created: ${_ORIGINAL}")
  else()
    message(WARNING "Link not created (${_RESULT})")
  endif()
endif()
