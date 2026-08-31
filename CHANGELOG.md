# Changelog

All notable changes to SBK_BarDrive will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

Use this section for changes made after the 2.1.0 development baseline.
Future architectural ideas are tracked separately in [ROADMAP.md](ROADMAP.md).

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
