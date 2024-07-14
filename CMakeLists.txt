
cmake_minimum_required (VERSION 3.25)

project (extra VERSION 0.0.1 LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)

include(GNUInstallDirs)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-config)

install(
    EXPORT ${PROJECT_NAME}-config
    NAMESPACE ${PROJECT_NAME}:: 
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

add_subdirectory(tests)
