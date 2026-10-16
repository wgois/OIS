# Cocoa focus reset regression

`run-cocoa-focus.py` builds a headless Objective-C++ regression against the OIS
library. Configure/build OIS first, then run from the source directory:

```sh
cmake -S . -B build -DOIS_BUILD_DEMOS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 1
python3 tests/mac/run-cocoa-focus.py --build-dir build
```

The runner accepts `CXX` and `CXXFLAGS` for local SDK/toolchain configuration.
It uses C++11, Cocoa, and the built OIS library; it creates no application,
window, view, event loop, or input device. No game or cursor API is called.

The keyboard uses the real CocoaInputManager constructor (not `_initialize`),
a real CocoaKeyboard with the manager's nil window, and its production responder,
injection, modifier callback, notifications, polling, capture, and teardown.
The HID manager constructor/destructor are empty; HID initialization is not used.

The mouse fixture extracts the production responder ivars and complete
`observeWindow:`, `detach`, `dealloc`, `setOISMouseObj:`, `focusLost:`,
`deliverButton:pressed:`, `capture`, six button handlers, `mouseMoved:`, and
`scrollWheel:` bodies unchanged. They run on an NSObject with real OIS mouse
state/events/listeners, a uniquely named minimal C++ Mouse subclass, and small
Foundation event-value objects. Its CocoaMouse name is a local type alias,
not the production C++ class. The NSView initializer/destructor and live cursor
paths are never invoked. This checks production state/callback logic; native
NSView construction and the actual CocoaMouse translation unit require separate
compilation/link validation.

Checks include queued key/repeat text discard; held keys/modifiers; immediate
polling reset; repeated loss; releases before fresh input; late physical ups;
null/unbuffered/replaced listeners; bounded mouse queue/overflow; preserved mouse
absolute position/dimensions; clearing motion/scroll; notification registration
and destruction; reentrant reset/capture; and detachment during release.
Window observer filtering and native responder/view attachment lifetime are
inspected in source, not exercised with a window.

## Focus and callback contract

- Observe application resign-active and only the manager's exact window
  resign-key notification. A nil window is not a wildcard window subscription.
  Both devices remove observers and detach their owner before responder release;
  the mouse also removes its view from the window. A retained observed window
  remains valid through explicit device teardown. Detach is idempotent.
- Notifications clear polling keys/modifiers or mouse buttons/relative axes and
  temporary motion immediately. Mouse absolute axes and dimensions survive.
  Pending keyboard presses/repeats and deferred mouse button snapshots are
  discarded. Repeated losses do not duplicate listener releases.
- Only held controls actually delivered to the current listener are balanced.
  Releases run once at the next capture, in ascending key/button order, before
  post-loss events. Keyboard releases carry zero text. Mouse release snapshots
  contain no held buttons or relative motion. Polling reports the latest physical
  input, so a newly pressed control may already poll down during an older
  synthetic release callback; the event describes its position in callback order.
- Ordinary mouse buttons keep their existing native callback timing. Between a
  loss and capture they use at most 32 copied state snapshots so old releases
  precede new presses. Overflow discards undelivered snapshots and resets polling
  buttons/motion again, retaining releases owed for delivered buttons. Fresh input
  can then queue normally; the next capture drains it. At most the queue length
  present when draining starts is delivered in one capture, so callbacks cannot
  refill it forever. Keyboard also drains a finite snapshot of its queue length.
- A replacement/null listener or unbuffered mode discards callback bookkeeping;
  releases are never sent to a different listener. Normal OIS listener ownership
  rules still apply: callers must remove a listener before destroying it.
- Capture retains the responder, removes each event/release before calling the
  listener, and stops its current drain if reset/detachment/listener replacement
  occurs. Nested capture returns immediately. New events added by callbacks can
  be delivered on the next capture.
- Native event dispatch and capture are assumed to use the same thread, as the
  existing backend does. Cocoa controls native routing after loss/reacquisition;
  this patch does not force focus or infer currently held hardware state.
  Physical focus restoration and live native delivery need a separate GUI test.

An application may clear its own effective/UI state on focus loss. It should
not add another synthetic OIS release source. The backend release policy works
without an application-specific reset, and this regression does not validate
any application's UI integration.

# Cocoa fractional motion regression

Configure OIS first to generate `includes/OISPrereqs.h`, then run on macOS with
Python 3 and the native C++ compiler:

```sh
cmake -S . -B build -DOIS_BUILD_DEMOS=OFF
python3 tests/mac/run-cocoa-motion.py
```

`CXX` and `CXXFLAGS` can supply a compiler, target architecture, or SDK include
settings. `--output-dir PATH` retains the generated source and executable;
otherwise they are removed after the run. `--source-root PATH` tests a separate
configured OIS checkout, for example the parent revision. A compile error or
failed expectation returns a nonzero exit status. No OIS library build or install
is required for this fixture.

The script extracts the production responder instance variables and the complete
`capture`, four movement/drag, `scrollWheel`, and `mouseEntered` method bodies
from `CocoaMouse.h` and `CocoaMouse.mm`. Unexpected method signatures or a missing
responder declaration stop extraction. The bodies are compiled unchanged into a
synthetic Foundation `NSObject` responder using the real OIS `MouseState` type.
Event values, window geometry, and mouse callback plumbing are small test doubles.
Cursor calls in `mouseEntered` are replaced with local no-op macros. The real
`NSView` initializer is not extracted or called. The fixture creates no AppKit
window or view, instantiates no OIS input manager, and never calls a live cursor
or input API.

The expectations cover all four handlers, positive and negative subunit motion,
mixed axes and handlers, signed reversal within and across captures, fractional
carry through empty captures, integer motion and absolute clamps, callback gates,
unchanged buttons and scroll/Z values, rejected warp/focus events, and explicit
position reset. They fail against the original per-event integer accumulation.

The implementation accumulates accepted X/Y deltas in private doubles. On each
capture it emits the integer part by truncating toward zero and subtracts exactly
those emitted counts, keeping the signed remainder for later captures. Opposing
events cancel before conversion. No rounding occurs per event. A normal entry
position seed discards pending whole and fractional motion, matching its existing
replacement of the temporary X/Y values; a rejected warp or focus event adds
nothing and leaves previously accepted motion intact. The original integer entry
position conversion, absolute integration/clamps, and scroll behavior are retained.

This is a bounded production-fragment regression. It proves the numeric and branch
behavior of those fragments under deterministic inputs. It does not exercise the
actual Cocoa responder, native event delivery, real windows, focus transitions,
cursor association/warping, display coordinate conversion, or game camera behavior.

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
