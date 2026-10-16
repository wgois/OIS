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
