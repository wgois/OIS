# Cocoa focus, motion, and canvas regression tests

Configure with CMake and run the registered tests from the build directory:

```sh
cmake -S . -B build -DOIS_BUILD_DEMOS=OFF -DBUILD_TESTING=ON
cmake --build build --parallel 3
ctest --test-dir build -C Release --output-on-failure
```

The portable MouseState and canvas-coordinate contract tests are built on every platform. On macOS, CTest builds the headless Objective-C++ production-fragment regression and modifier regression. The generated fixture is refreshed when the production CocoaMouse header or implementation changes. It uses deterministic Foundation values and never creates a window, initializes live input, or calls cursor APIs.
