
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

add_executable(extra_tests 
	"sandbox.cpp"
	"trait/abuse_nested_types.cpp" 
	"trait/regular_specialization.cpp" 
	"trait/adl_bridge.cpp"  
	"trait/transparent_call.cpp")

target_compile_features(extra_tests 
	PUBLIC cxx_std_20)

target_link_libraries(extra_tests 
	PRIVATE extra Catch2::Catch2WithMain)

catch_discover_tests(extra_tests)
