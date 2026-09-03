# Changelog

All notable changes to SBK_BarDrive will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

Use this section for changes made after the 2.1.2 release.
Future architectural ideas are tracked separately in [ROADMAP.md](ROADMAP.md).

## [2.1.2] - 2026-09-03

### Added

- Universal `forTime()` modifier for setting a hard maximum duration on direct
  animations or queued entries. It may be placed immediately before or after
  `enqueue()`.
- `enqueueFor()` convenience API for time-limited queued animations.
- Parameterless `enqueueReverseAnim()` for a state-preserving logic reversal of
  the preceding queued animation. Standard `loop()`/`noLoop()`, `forTime()`, and
  `stopBlockEmissionAfter()` modifiers configure the reversed entry.
- `stopBlockEmissionAfter()` for direct or queued mirrored and scrolling block
  animations. New emissions stop at the deadline while active blocks drain
  naturally before queue advancement.

### Changed

- `wait()` is always non-looping, preventing an inherited loop setting from
  indefinitely blocking subsequent queue entries.
- `animationQueueDemo` now documents hard deadlines, state-preserving reversal,
  graceful block draining, modifier ordering, and automatic queue replay.

## [2.1.1] - 2026-08-31

### Fixed

- Initialize the logical segment count in the generic rows-by-columns matrix
  constructor. Generic matrix bars now accept pixel updates and animations
  instead of behaving as zero-segment displays.

### Added

- `genericMatrixDemo` example covering the generic rows-by-columns constructor.

## [2.1.0] - 2026-08-31

### Added

- Fixed-storage animation queue with `enqueue()`, `startQueue()`,
  `clearQueue()`, and `skipCurrent()`.
- Compile-time `SBK_BARDRIVE_QUEUE_CAPACITY` setting, defaulting to four.
- Queue state inspection through `queuedAnimations()`, `isQueueRunning()`,
  `queueOverflowed()`, `currentQueueIndex()`, and
  `isQueueIndexPlaying()`.
- `NO_QUEUE_INDEX` sentinel for detecting that no queued entry is active.
- Non-blocking `wait(duration)` animation for timed queue pauses without
  changing pixel states.
- `blinkPixel()` overloads with fixed or independent on/off intervals.
- `setUpdateInterval()` for changing an active animation's primary interval.
- `animationQueueDemo` example for queue construction, live logic inversion,
  graceful block-emission shutdown, and `wait()`.
- Root-level `Doxyfile` for reproducible API documentation generation.

### Changed

- `stop()` now terminates looping and queued work and clears mapped pixels;
  `pause()` remains the non-destructive way to freeze an animation.
- Block and random animations share one construction-time workspace instead
  of allocating separate buffers.
- Animation workspace capacity is derived from the bar segment count and
  bounded to 64 simultaneous blocks.
- Generic live-value pointers share storage where their lifetimes do not
  overlap.
- Fixed-value `beatPulse()` calculates its interval during setup; the live-BPM
  overload remains available for pointer-driven BPM changes.
- Duration-based bounce interval calculations now use integer arithmetic only.
- Documentation and examples now use current driver class names and
  `MatrixPreset` terminology.
- Doxygen HTML documentation regenerated from the 2.1.0 API and examples.
- Minimum driver releases updated to SBK_HT16K33 2.0.1 and
  SBK_MAX72xx 2.0.5.

### Fixed

- Corrected a duplicated lower-bound test in block tracking.
- Corrected the `uint16_t` maximum-value check from an accidental comma
  expression to `65535`.
- Corrected block-switch helper names and related comments.
- Queue index state is cleared consistently when a queue stops, finishes, or
  is cleared.

## Older versions

Detailed release notes were not maintained in this format before 2.1.0.
Refer to the repository history and release tags for earlier changes.
