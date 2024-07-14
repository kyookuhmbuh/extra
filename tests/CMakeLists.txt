
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(extra_tests 
	"trait/trait_from_nested_types.cpp" 
	"trait/trait_from_specialization.cpp" 
	"trait/trait_from_adl_bridge.cpp")

target_compile_features(extra_tests 
	PUBLIC cxx_std_20)

target_link_libraries(extra_tests 
	PRIVATE extra Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(extra_tests)
