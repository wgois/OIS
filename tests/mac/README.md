# Regression tests

From the repository root, configure and build OIS, then run its registered tests:

```sh
mkdir -p build
cd build
cmake .. -DOIS_BUILD_DEMOS=OFF -DBUILD_TESTING=ON
cmake --build . --config Release
ctest -C Release --output-on-failure
```

The portable MouseState contract test is built on every platform. On macOS, CMake also builds and registers the headless Cocoa Objective-C++ regression as a normal target. The generated fixture is refreshed when its production header or implementation changes. It uses deterministic Foundation values and does not create windows or initialize live input.
