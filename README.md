# Extra Utilities Library

`extra` is a header-only C++ utility library providing a collection of small, reusable tools and helpers.

## Usage

Add the library to your project (e.g., via CPM or FetchContent)

```cmake
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# create project
project(MyProject)

# add executable
add_executable(main main.cpp)

# add dependencies
include(FetchContent)
FetchContent_Declare(
  extra
  GIT_REPOSITORY https://github.com/kyookuhmbuh/extra.git
)
FetchContent_MakeAvailable(extra)

# link dependencies
target_link_libraries(main extra::extra)
```

and include the main header:

```cpp
#include <extra/extra.hpp>
```

See [`include/extra/extra.hpp`](include/extra/extra.hpp) for available modules.

## License

Distributed under the MIT License.
