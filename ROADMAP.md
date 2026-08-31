# SBK_BarDrive Roadmap

This document records possible future directions. Items listed here are ideas,
not commitments, and may change after profiling and hardware testing.

## 2.1.x: stabilization

The 2.1.x series should favor compatibility and predictable embedded behavior:

- Complete testing of queue transitions, skipping, looping, and overflow.
- Test every supported driver setup and mapping constructor.
- Keep runtime animation processing free of dynamic allocation.
- Measure flash and SRAM on ATmega328P and ATmega4809 after meaningful changes.
- Correct defects and documentation without redesigning the public API.
- Regenerate the Doxygen HTML documentation from the current source before a
  published release.

## 3.0.0: possible internal architecture

Version 3.0.0 may reorganize the animation engine while preserving the useful
chainable API where practical.

### Typed active animation state

Replace generic active fields such as `_param1`, `_counter1`, and
`_ledTracker1` with clearly named state structures:

```cpp
struct BlockState;
struct FillState;
struct BlinkState;
struct SignalState;
```

Only one animation runs at a time, so these structures could share storage in
a union. This should improve readability without automatically increasing
SRAM.

### Separate configuration, runtime, and progress

Clearly distinguish:

- Animation configuration: type, intervals, direction, logic, and looping.
- Runtime control: running, paused, initialized, and last-update time.
- Animation-specific progress: positions, counters, peaks, and active blocks.

Every animation should follow one lifecycle:

```text
configure -> enqueue or start -> initialize -> update -> finish
```

### Compact queue descriptors

Queue entries could store only the information required to initialize an
animation instead of snapshotting most of the controller state. The active
typed state would be initialized when the entry starts.

The queue must remain fixed-storage and compile-time configurable by default.
No allocation should occur while animations are running.

### Explicit animation types and dispatch

Introduce an `AnimationType` enum for queue descriptors and active-state
selection. Compare a `switch` dispatcher with the current member-function
pointer dispatcher on AVR, retaining whichever produces the better flash and
runtime result.

### Compact flag storage

Evaluate explicit bit-mask flags for loop, logic, direction, and pointer modes.
Do not use C++ bitfields without measurement: the 2.1.0 experiment reduced
SRAM but increased AVR flash enough to be rejected.

### Internal file organization

Consider separating the animation implementation into focused template headers:

```text
SBK_BarMeterAnimations.h
internal/SBK_AnimationTypes.h
internal/SBK_AnimationQueue.h
internal/SBK_AnimationStates.h
internal/SBK_BlockAnimations.h
internal/SBK_FillAnimations.h
internal/SBK_SignalAnimations.h
```

This is an organizational change, not a reason to duplicate state or force all
animation code into every firmware image.

### Workspace ownership options

Investigate alternatives to the construction-time heap allocation used by the
shared block/random workspace:

- Compile-time internal storage.
- User-supplied external storage.
- A capacity template parameter or build macro.

Any alternative should preserve universal display sizes and avoid heap
activity during operation.

## Evaluation requirements for 3.0.0

Architectural changes should be accepted only after comparison with 2.1.x:

- Flash and SRAM on ATmega328P.
- Flash and SRAM on ATmega4809.
- Queue-entry size and controller-object size.
- No heap allocation after construction.
- Visual equivalence of existing animations.
- Compatibility tests for MAX72xx software SPI, MAX72xx hardware SPI, and
  HT16K33.
- Example-sketch compilation for all supported configurations.

Readability improvements are important, but they must not silently make the
library unsuitable for constrained AVR targets.
