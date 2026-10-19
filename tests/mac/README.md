# Cocoa focus reset regression tests

Configure with CMake and run the registered tests from the build directory:

```sh
cmake -S . -B build -DOIS_BUILD_DEMOS=OFF -DBUILD_TESTING=ON
cmake --build build --parallel 3
ctest --test-dir build -C Release --output-on-failure
```

The portable MouseState contract test is built on every platform. On macOS, CTest also builds the headless Objective-C++ regression from the production Cocoa method bodies. The generated fixture is refreshed when its source header or implementation changes. It uses deterministic Foundation values and never creates a window, initializes live input, or calls cursor APIs. The fixture checks production method behavior within its test doubles; native Cocoa event delivery remains outside its scope.
