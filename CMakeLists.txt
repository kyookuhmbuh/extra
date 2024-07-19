
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

if (BUILD_TESTING)
	include(FetchContent)

	FetchContent_Declare(
	  Catch2
	  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	  GIT_TAG        v3.4.0 # or a later release
	)

	FetchContent_MakeAvailable(Catch2)

	list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
	include(CTest)
	include(Catch)
	enable_testing()

	add_executable(${PROJECT_NAME}_tests 
		"tests/sandbox.cpp"
		"tests/trait/abuse_nested_types.cpp" 
		"tests/trait/regular_specialization.cpp" 
		"tests/trait/adl_bridge.cpp"  
		"tests/trait/transparent_call.cpp")

	target_compile_features(${PROJECT_NAME}_tests 
		PUBLIC cxx_std_20)

	target_link_libraries(${PROJECT_NAME}_tests 
		PRIVATE ${PROJECT_NAME} Catch2::Catch2WithMain)

	catch_discover_tests(${PROJECT_NAME}_tests)
endif()
