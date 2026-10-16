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
