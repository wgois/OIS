# Cocoa keyboard modifier regression

`CocoaKeyboardModifiers.mm` sends synthetic `NSEvent` flags directly to the
production `CocoaKeyboardView::flagsChanged:` implementation. It checks modifier
masks, polled keys, copied key states, and buffered listener callbacks. No
application, window, event loop, or HID device enumeration is needed. The test
constructs the Cocoa input manager without calling its `_initialize()` method.

Run on macOS from the repository root with the Command Line Tools and CMake:

```sh
build_dir="../ois-build-release"
sdk_path="$(xcrun --sdk macosx --show-sdk-path)"
cmake -S . -B "$build_dir" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_SYSROOT="$sdk_path" \
  -DOIS_BUILD_DEMOS=OFF
cmake --build "$build_dir" --parallel 4
xcrun clang++ -std=c++11 -arch arm64 -isysroot "$sdk_path" \
  -I includes tests/mac/CocoaKeyboardModifiers.mm \
  -L "$build_dir" -lOIS -framework Cocoa \
  -Wl,-rpath,@loader_path -o "$build_dir/cocoa-keyboard-modifiers"
"$build_dir/cocoa-keyboard-modifiers"
```

If the local toolchain cannot find standard C++ headers, add
`-DCMAKE_CXX_FLAGS="-isystem $sdk_path/usr/include/c++/v1"` to the CMake command
and `-isystem "$sdk_path/usr/include/c++/v1"` to the compiler command. This is a
local toolchain workaround, not required OIS configuration.

The fixture covers presses from zero, releases preserving other held modifiers,
device-dependent native bits, simultaneous and mixed portable transitions,
repeated Caps Lock/NumericPad flag cycles, Command/Function delivery, and
unbuffered state updates. A successful run prints `65 checks, 0 failures`.

The backend continues reporting aggregate Shift/Option/Control/Command flags
using left key codes; the fixture does not establish independent left/right
tracking. NumericPad retains the existing `KC_NUMLOCK`/`NumLock` alias, with its
state following the native flag. This does not imply a physical Num Lock key or
validate actual hardware event delivery. Native application focus and real
keyboard integration require separate testing.
