# Simple Timer

A lightweight, header-only C++ library for measuring code execution time. It supports C++17 and newer, handles both `void` and returning functions, and includes optional MPI support.

## Features

- **Header Only:** No binary dependencies (unless using MPI).
- **Cross-Platform:** Works on any platform supporting C++17 (Windows, Linux, macOS).
- **Flexible:** Supports functions, lambdas, and curried arguments.
- **Type Safe:** automatic deduction of return types (`TimeResult<T>` vs `TimeResult<void>`).
- **MPI Support:** Optional integration with `MPI_Wtime` for high-performance computing environments.

## Installation

### Manual
Copy `timer.hpp` into your project's include directory.

### CMake (FetchContent)
Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    simple-timer
    GIT_REPOSITORY https://github.com/Matthew-Krueger/simple-timer.git
    GIT_TAG        main 
)

FetchContent_MakeAvailable(simple-timer)

target_link_libraries(my_executable PRIVATE simple-timer::simple-timer)
```

## Critical Note on Precision and Truncation

This library defines its own duration types within the `timer` namespace (e.g., `timer::seconds`) which use **double precision**.

However, standard `std::chrono` types (e.g., `std::chrono::seconds`) usually use **integers**.

**If you cast a short duration to an integer-based unit, it will truncate to zero.**
Using the getDurationView function, you can access the underlying double precision value and cast it independently, such as if you need a float or a double or an unsigned integer.

| Unit Type              | Behavior               | Example Input  | Result |
|:-----------------------|:-----------------------|:---------------|:-------|
| `timer::seconds`       | **Double Precision**   | 0.75 seconds   | `0.75` |
| `std::chrono::seconds` | **Integer Truncation** | 0.75 seconds   | `0`    |

## Usage

### Basic Usage

The `timer::time` function accepts a callable (function pointer or lambda).

```cpp
#include <iostream>
#include <thread>
#include "timer.hpp"

using namespace std::chrono_literals;

void heavyWork() {
    std::this_thread::sleep_for(200ms);
}

int main() {
    // 1. Time a void function
    auto result = timer::time(heavyWork);

    // Access the raw count (double precision seconds by default)
    std::cout << "Time: " << result.duration.count() << " seconds\n";
    
    return 0;
}
```

### Retrieving Return Values

If the measured function returns a value, `timer::time` returns a `TimeResult<T>` containing both the result and the duration.

```cpp
int calculate() {
    return 42;
}

int main() {
    auto result = timer::time(calculate);

    std::cout << "Function returned: " << result.functionResult << "\n";
    std::cout << "Time taken: " << result.duration.count() << "s\n";
}
```
If for some reason the compiler incorrectly deduces the return type as `TimeResult<void>`,
double-check there are no other definitions of the function, and that the function returns a value.
You may manually specify the type via the template parameter using std::declval<std::invoke_result_t<F>>, where F is the function signature.

### Passing Arguments (Currying)

To time a function that requires arguments, wrap the call in a lambda.

```cpp
void processData(int id, int value) {
    // ... processing
}

int main() {
    int id = 1; 
    int val = 100;

    auto result = timer::time([id, val]() { 
        return processData(id, val); 
    });
}
```

### Advanced Duration Access

The library provides `getDurationView` and `as<T>` for precise unit conversion.

```cpp
// 1. Recommended: Direct double access (Seconds, no truncation)
double sec = result.duration.count(); 
double sec = result.getDurationCount();

// 2. Using timer units (Double precision, no truncation)
double ms = result.getDurationView<timer::milliseconds>().as<double>();
double us = result.getDurationView<timer::microseconds>().as<double>();

// 3. Using std::chrono units (WARNING: Truncates to integer due to C++ spec)
// 0.9 seconds becomes 0 seconds here:
auto s_int = result.getDurationView<std::chrono::seconds>().as<int>(); 
```

## MPI Support

To use the MPI wall clock (`MPI_Wtime`), define the build flag before including the header or via your compiler arguments.

**Compiler Flag:**
`-DBUILD_WITH_MPI`

**In Code:**
```cpp
#define BUILD_WITH_MPI
#include "timer.hpp"
```

*Note: You must ensure your application links against the MPI library correctly.*

## HPC Specifics
If an HPC compiler throws an error about `std::chrono::high_resolution_clock` not being defined, please link and compile against MPI. `srun` invokes an MPI context anyway, so it really doesn't matter.
If an HPC compiler throws an error about MPI_Wtime not being defined, even if not building with MPI, turn on MPI building and link with MPI. `srun` invokes an MPI context anyway, so it really doesn't matter.
On an HPC system, there is some argument over whether MPI_Wtime is enough. This implementation does *not* use MPI_Barrier, however arguably this should happen. This may be added in a future update.

## Additional Demos
See `main.cpp` for additional examples.

## Dependency Specifics
The library has no external dependencies outside of the standard library, C++17 or above.
The library will make use of `MPI_Wtime` if the build flag is defined.
The library will make use of some limited compiler intrinsics if compiling with GCC or Clang.

## License
Copyright 2026 Matthew Krueger <contact@matthewkrueger.com>
Original Source: https://github.com/Matthew-Krueger/simple-timer

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
