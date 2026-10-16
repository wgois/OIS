# Cocoa headless regressions

These changes depend on the reviewed focus-reset (#103), fractional-motion
(#102), and modifier (#101) proposals. They do not imply those proposals have
merged into upstream master.

Configure/build OIS, then run the combined regression and modifier regression on
macOS with Python 3, CMake, and the Command Line Tools:

```sh
cmake -S . -B build -DOIS_BUILD_DEMOS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 1
python3 tests/mac/run-cocoa-focus.py --build-dir build
clang++ -std=c++11 -I includes tests/mac/CocoaKeyboardModifiers.mm \
  -L build -lOIS -framework Cocoa -Wl,-rpath,@loader_path \
  -o build/cocoa-keyboard-modifiers
build/cocoa-keyboard-modifiers
```

`CXX` and `CXXFLAGS` configure the Python runner's compiler. If the local compiler
cannot find the C++ standard library headers, set CMake's `CMAKE_CXX_FLAGS` and the
fixture's `CXXFLAGS` to `-isystem <SDK>/usr/include/c++/v1` using the actual SDK path.
The combined runner writes its exact compiler arguments to `fixture-command.json`.

For the mouse/coordinate checks alone, no OIS library build is needed after CMake
has generated `includes/OISPrereqs.h`:

```sh
python3 tests/mac/run-cocoa-focus.py --build-dir build --mouse-only
# The previous motion entry point runs the same shared mouse fixture:
python3 tests/mac/run-cocoa-motion.py
```

The mouse-only runner reports 225 checks. The combined runner adds the original
20 keyboard focus checks (245 total). `CocoaKeyboardModifiers.mm` reports 65
checks. The old separate motion fixture is replaced by the shared fixture:
compatible fractional-motion cases run alongside the focus and coordinate cases,
with current expectations for event-time absolute positions and position-only
movement callbacks. No claim is made that the unchanged old 110-case fixture runs.

## What actually runs

The keyboard uses the real CocoaInputManager constructor (not `_initialize`),
a real CocoaKeyboard with a nil manager window, and its production responder,
injection, modifier callback, notifications, polling, capture, and teardown.
The HID manager constructor/destructor are empty; HID initialization is not used.

The mouse fixture extracts the production responder ivars and complete
`observeWindow:`, `detach`, `dealloc`, `setOISMouseObj:`, `focusLost:`,
`deliverButton:pressed:`, `capture`, six button handlers, all four movement/drag
handlers, `updateMotion:`, `refreshCoordinates`, `seedCoordinates:`, and
`scrollWheel:` bodies unchanged. They run on an NSObject with real OIS mouse
state/events/listeners and a minimal C++ Mouse subclass. Small Foundation objects
supply event values; the NSObject supplies a raw bounds rectangle and flipped flag.
Native point conversion is declared but never invoked. Captured event values return a constant absolute position and count reads; the
checks assert it is never read, independently of window style. The production coordinate helper is also tested directly.

No NSView, NSWindow, NSApplication, event loop, input manager initialization,
input device, GPU, or live cursor API is used by the mouse fixture. Its
`restoreAssociation` is a local teardown stub; the actual association request,
seeding through `convertPoint:fromView:nil`, tracking area/autoresizing, native
view attachment, and teardown require real translation-unit compilation and source
review, with live behavior still unverified. The fixture does not emulate AppKit.

## Coordinate and callback contract

The private Cocoa helper retains a floating top-origin virtual cursor in logical
view units. Application canvas width/height divided by actual input-view bounds
size determine independent axis ratios; screen backing scale is not used.
The deliberate captured policy is one native delta unit per logical cursor unit.
Relative camera X/Y counts remain unscaled and retain signed fractional carry
across captures. This is a policy, not a physical-speed measurement.

A valid associated window position is converted into the input view before
backend disassociation. Bounds origin and flippedness normalize that point.
When a usable seed/geometry is unavailable, the first valid geometry supplies a
center fallback. Initialization and re-entry clear pending relative movement,
without injecting seed coordinates as relative camera motion. There is no cursor
warp. Only successful backend disassociation is tracked; exit/destruction attempt
to restore it. Failure to restore retains ownership so teardown can retry. This
assumes other hosts do not independently change global CoreGraphics association.

Move/drag events integrate absolute position immediately. Button events copy the
current absolute position with zero X/Y/Z relative fields, preserving normal native
callback timing and preventing capture motion or wheel replay. Deferred button
snapshots additionally retain a private floating event-time logical point. Before
each callback, including after an earlier callback changed geometry, that point is
projected into the current valid canvas with current dimensions and zero REL.
Historical points can lie outside bounds after a shrink; the live cursor clamps,
while queued positions retain history (integer overflow alone saturates). Capture publishes
relative/wheel values once and reports changed integer absolute positions even
when relative counts are zero. Canvas-only changes republish absolute coordinates
without camera motion. A view resize clamps the logical cursor once. Invalid
zero/nonfinite geometry suppresses conversion until valid, preserving the previous
published absolute position. The inclusive OIS width/height clamp is retained.

Focus loss clears buttons, pending relative motion/carry, movement notification,
and deferred snapshots immediately, retaining the virtual cursor and geometry.
Only controls actually delivered to the current listener receive balancing releases
at the next capture, before fresh deferred input. Invalid geometry defers balancing
releases and queued transitions without popping them, and capture returns after
one bounded attempt. New buttons wait in the same FIFO; its existing 32-event
overflow/reset, listener replacement and focus-reset rules still apply. A queued
event arriving while geometry is invalid retains the last known logical cursor
(initially zero until a valid seed). Recovery delivers in order against each
callback's current geometry. Repeated loss, overflow, listener replacement/null listeners,
unbuffered mode, nested capture, reentrant reset, and detachment retain the focus
proposal's guards. Native dispatch and capture are assumed to share one thread.

Tests cover 1, 2, and 1.5 canvas ratios, independent axes, nonzero origin/flipped
seeds, invalid geometry, all captured movement/drag paths, signed fractional carry,
reversal, focus gating/reset, density and bounds changes, move-button-release
before capture, capture then button, wheel consumption, deferred snapshots spanning
density/view changes, callback-induced geometry changes, invalid geometry
retention/recovery, and existing focus/listener/reentrancy cases. The tests and compile checks cannot prove
physical input speed, real focus/re-entry, pointer association success, external
transformed host geometry, monitor transitions, or application UI behavior.

The modifier fixture uses synthetic NSEvent values with production callbacks. It
covers all portable modifier transitions, preserving other held flags, native bits,
CapsLock/NumericPad flag cycles, Command/Function, and polling/unbuffered paths.
Aggregate Shift/Option/Control/Command still use left key codes; independent
left/right hardware behavior and actual keyboard delivery remain unverified.
