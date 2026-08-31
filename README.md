# SBK\_BarDrive Library

High-level Arduino library for controlling animated LED bar meters using MAX7219/MAX7221 or HT16K33 drivers. Ideal for prop-making, visual meters, and signal-driven lighting effects.

---

## What's New in 2.1.1

Version 2.1.1 fixes the generic rows-by-columns matrix constructor so it
initializes the logical segment count. Generic matrix displays now work with
pixel operations and animations as documented.

## What's New in 2.1.0

Version 2.1.0 adds configurable, fixed-storage animation queues, queue index
inspection and skipping, non-blocking `wait()`, single-pixel blinking, safer
animation workspace allocation, and AVR-focused RAM and flash optimizations.
See [CHANGELOG.md](CHANGELOG.md) for the complete development history.
Potential future architecture work is recorded in [ROADMAP.md](ROADMAP.md).

Versions prior to 2.0.0 remain deprecated because of internal changes to
offset handling and multi-device support.

---


## ✨ Features

* Unified API for both **SPI** (MAX72xx) and **I2C** (HT16K33) LED drivers
* Built-in **bar meter animations** (e.g., filling, bouncing, signal-following, block effects)
* Fixed-storage, non-blocking **animation queues**, configurable at compile time
* Support for **custom \[row, col] segment mappings** or preset types
* Compile-time options to optimize for memory:

  * `SBK_BARDRIVE_WITH_ANIM` to include animations only if desired
  * `SBK_BARDRIVE_QUEUE_CAPACITY` to select the queue capacity (default: `4`)
* Designed for **SBK BarMeter** and **SBK BarDrive** PCBs
* Reverse display modes and flexible mapping
* Internal buffer with batch `.show()` updates
* **Software SPI (MAX72xx)** — works on any 3 digital pins: `DATA`, `CLK`, and `CS`
* **I2C (HT16K33)** — uses `SDA` and `SCL` pins (standard I2C bus)

---

## ⚙️ Supported Hardware Combinations

This library is compatible with **any LED matrix or bar display** using `MAX7219`, `MAX7221`, or `HT16K33` drivers — as long as a valid `[dev, row, col]`, aka `[device index, anode,cathode]`, segment mapping is provided or configured using a built-in preset.

When using custom segment mappings with SBK_BarDrive, each segment is defined as a {dev, row, col} — representing the physical LED connection:

{dev, row, col} = {device index, anode, cathode}

This matches the wiring convention of common LED driver ICs:
| Driver  | Row (Anode / V+)    | Column (Cathode / GND) |
| ------- | ------------------- | ---------------------- |
| MAX72xx | `SEGx` (source, V+) | `DIGx` (sink, GND)     |
| HT16K33 | `Rx`  (source, V+)  | `Cx`   (sink, GND)     |


### 🧩 SBK Bar Meter Boards & Accessories

SBK offers purpose-built PCBs to simplify bar meter wiring, driver integration, and display control. These include:

- **Bar meter carriers** with preset mappings for 28-segment and modular bar displays
- **Driver interface boards** for MAX7219/MAX7221 (SPI) and HT16K33 (I²C)

Some models are available via the [**SBK Tindie Store**](https://www.tindie.com/stores/smartbuildskits/) or upon request by emailing [SmartBuildsKits@gmail.com](mailto:SmartBuildsKits@gmail.com).

📁 For schematics, 3D models, and PCB source files, visit:  
🔗 [https://github.com/sbarabe/SBK_PCBs](https://github.com/sbarabe/SBK_PCBs)

---


## 📦 Dependencies

The `SBK_BarDrive` library **depends on one of the following display driver libraries** to function:

| Dependency         | Description                                                              | Required For                     |
|--------------------|---------------------------------------------------------------------------|----------------------------------|
| [`SBK_MAX72xx`](https://github.com/sbarabe/SBK_MAX72xx) | Software SPI driver for MAX7219/MAX7221 LED drivers               | MAX72xx-based displays           |
| [`SBK_HT16K33`](https://github.com/sbarabe/SBK_HT16K33) | I²C driver for HT16K33 LED driver (8x16 matrices or bar displays) | HT16K33-based displays           |

You **must install at least one** of these drivers depending on your hardware.

> If using PlatformIO or Arduino Library Manager, these will be installed automatically as dependencies.

To manually install:
```bash
# For MAX72xx (SPI)
git clone https://github.com/sbarabe/SBK_MAX72xx.git

# For HT16K33 (I2C)
git clone https://github.com/sbarabe/SBK_HT16K33.git
```

Then place them in your Arduino `libraries` folder.


## ⬇️ Installation

1. Download or clone the library into your Arduino `libraries` folder:

   ```bash
   git clone https://github.com/sbarabe/SBK_BarDrive.git
   ```
2. In your Arduino sketch, enable features as needed:

   ```cpp
   #define SBK_BARDRIVE_WITH_ANIM
   // Optional: defaults to 4. Must be defined before SBK_BarDrive.h.
   #define SBK_BARDRIVE_QUEUE_CAPACITY 2
   #include <SBK_BarDrive.h>
   ```

### Animation working memory

Each `SBK_BarDrive` animation controller sizes its working buffers from the
actual number of segments and allocates them once during construction. The
same buffers are reused by every animation, so changing animations does not
allocate or free heap memory. Block storage scales with the bar size and keeps
the existing maximum of 64 simultaneous blocks.

---

## 🔊 Quick Start Examples

See [`animationQueueDemo`](examples/animationQueueDemo/animationQueueDemo.ino)
for a four-entry queue, live logic inversion without restarting the active
animation, graceful block-emission shutdown, and a non-blocking `wait()` entry.

### Using MAX7219:

```cpp
#define SBK_BARDRIVE_WITH_ANIM

#include <SBK_MAX72xxSoft.h>
SBK_MAX72xxSoft max72xx(DATA_PIN, CLK_PIN, CS_PIN, 1);

#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_MAX72xxSoft> bar(&max72xx, 0, MatrixPreset::BL28_3005SK);

void setup() {
    max72xx.begin();
    bar.animations().animInit().fillUpIntv(50).loop();
}

void loop() {
    bar.animations().update();
    bar.show();
}
```

### Using HT16K33:

```cpp

#define SBK_BARDRIVE_WITH_ANIM

#include <SBK_HT16K33.h>
SBK_HT16K33 ht(1);

#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_HT16K33> bar(&ht, 0, 28);

void setup() {
    ht.setAddress(0,0x70);  // Set device I2C address
    ht.setDriverRows(0,8); // 20-SOP = 8 rows, 24-SOP = 12, 28-SOP = 16
    ht.begin();
    bar.animations().animInit().scrollingUpBlocks(60, 2, 1).loop();
}

void loop() {
    bar.animations().update();
    bar.show();
}
```
### Using a custom mapping :
If you want full control over how segments map to physical LED positions, you can supply a custom [device, row, col] mapping array. This is ideal for irregular layouts or displays spanning multiple devices.

```cpp

#define SBK_BARDRIVE_WITH_ANIM

// Define a custom segment-to-pixel mapping
const uint8_t mapping[5][3] = {
  {0, 0, 0},  // Segment 0 → Device 0, Row 0, Col 0
  {0, 0, 1},  // Segment 1 → Device 0, Row 0, Col 1
  {0, 0, 2},
  {1, 0, 0},  // Segment 3 → Device 1, Row 0, Col 0 (spans devices)
  {1, 0, 1}
};

// Instantiate driver and bar
#include <SBK_MAX72xxSoft.h>
SBK_MAX72xxSoft max72xx(DATA_PIN, CLK_PIN, CS_PIN, 2); // 2 devices

#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_MAX72xxSoft> bar(&max72xx, 0, mapping);

void setup() {
  max72xx.begin();
  bar.animations().animInit().fillUpIntv(60).loop();
}

void loop() {
  bar.animations().update();
  bar.show();
}
```

---


## 🎞️ Built-In Animations

These functions start animations and can be chained with modifiers like `.loop()` or `.pause()`.

### Fill / Empty
```cpp
fillUpIntv();       // Interval-based upward fill
fillDownIntv();     // Interval-based downward fill
fillUpDur();        // Duration-based upward fill
fillDownDur();      // Duration-based downward fill
emptyUpIntv();      // Interval-based upward empty
emptyDownIntv();    // Interval-based downward empty
emptyUpDur();       // Duration-based upward empty
emptyDownDur();     // Duration-based downward empty
```

### Bounce Effects
```cpp
bounceFillUpIntv();         // Bounce up (interval)
bounceFillDownIntv();       // Bounce down (interval)
bounceFillUpDur();          // Bounce up (duration)
bounceFillDownDur();        // Bounce down (duration)
bounceFillFromCenterIntv(); // Bounce from center outward (interval)
bounceFillFromCenterDur();  // Bounce from center outward (duration)
bounceFillFromEdgesIntv();  // Bounce from edges inward (interval)
bounceFillFromEdgesDur();   // Bounce from edges inward (duration)
```

### Block-Based Animations
```cpp
scrollingUpBlocks();        // Scroll blocks upward
scrollingDownBlocks();      // Scroll blocks downward
collidingBlocks();          // Emit mirrored blocks toward center
explodingBlocks();          // Emit mirrored blocks from center outward
upwardStackingBlocks();     // Launch blocks bottom to top and stack
downwardStackingBlocks();   // Drop blocks from top and stack bottom
upwardUnstackingBlocks();   // Launch blocks up and remove top stack
downwardUnstackingBlocks(); // Drop blocks down and unstack from bottom
upwardUnstackingBlocks();    // Launch blocks upward and unstack from top
downwardUnstackingBlocks();  // Drop blocks downward and unstack from bottom
```

### Signal-Driven
```cpp
followSignalSmooth();          // Smooth fill from signal
followSignalWithPointer();     // Fill + signal pointer
followDualSignalFromCenter();  // Mirror fill from center using 1 (mirrored) or 2 signals
followDualSignalFromEdges();   // Mirror fill from edges inward using 1 (mirrored) or 2 signals
followSignalFloatingPeak();     // Signal smoothing + floating peak indicator
```

### Random & Beat
```cpp
randomFill();     // Random pixel fill
randomEmpty();    // Random pixel clear
beatPulse();      // BPM-based pulse effect
```

### Static Setters
```cpp
setAllOn();       // Turn on all pixels
setAllOff();      // Turn off all pixels
setAll(bool);     // Conditionally turn all on/off
blinkPixel(0);                 // Blink pixel 0 continuously at 500 ms
blinkPixel(0, 250, 3);         // Blink pixel 0 three times at 250 ms
blinkPixel(0, 100, 400, 3);    // Three blinks with separate ON/OFF times
```

---

## 🧰 Animation Helpers (Chainable)

These methods allow you to control, manipulate, or conditionally alter animations at runtime. Most are **event-driven** triggers—ideal for reacting to user input, state transitions, or timed events.

You can chain these calls because they return a reference to the active animation controller:

````cpp
bar.animations().pause();
bar.animations().resume();
bar.animations().stop();
bar.animations().loop();
bar.animations().noLoop();
bar.animations().toggleLogic();
bar.animations().invertLogic();
bar.animations().resetLogic();
bar.animations().stopBlockEmission();
bar.animations().resumeBlockEmission();
````

`pause()` freezes the current animation and preserves both its state and the
visible pixels so that `resume()` can continue it. `stop()` fully terminates the
current animation, disables looping, clears the pending queue, and clears the
mapped pixels in the driver's buffer. Call `bar.show()` afterward when the
cleared buffer must be sent to the display immediately.

All helper functions return a reference to the `SBK_BarMeterAnimations` object, allowing chainable expressions like:

````cpp
bar.animations().pause().setLogic(true).resume().stopBlockEmission();

bar.animations()
    .pause()
    .setLogic(true)
    .resume()
    .stopBlockEmission();
````

### Animation queue

Up to `SBK_BARDRIVE_QUEUE_CAPACITY` configured animations can be queued without
dynamic allocation. The default capacity is four.
Configure an animation, apply any modifiers, and capture it with `enqueue()`:

```cpp
bar.animations()
    .clearQueue()
    .fillUpDur(500).enqueue()
    .emptyDownDur(750).enqueue()
    .fillDownDur(500).reverseDir().enqueue()
    .startQueue();
```

Calling `update()` automatically starts the next entry when the current one
finishes. `queuedAnimations()` reports the number still waiting,
`isQueueRunning()` reports whether a queue is active, and
`queueOverflowed()` reports an attempt to exceed the configured capacity.
`currentQueueIndex()` returns the physical queue slot currently playing, or
`NO_QUEUE_INDEX` when none is active. `isQueueIndexPlaying(index)` provides a
direct test, for example:

```cpp
if (animations.isQueueIndexPlaying(2))
    animations.skipCurrent();
```

Use `wait(duration)` to add a non-blocking delay that preserves the current
pixel states:

```cpp
animations.wait(1000).enqueue();
```
An individually looped or continuous animation intentionally prevents later
entries from starting automatically. Call `skipCurrent()` to abandon it and
immediately continue with the next queued animation. Unlike `stop()`, skipping
does not clear pending entries.

---

## 📘 API Overview

| Class                    | Purpose                                     |
| ------------------------ | ------------------------------------------- |
| `SBK_BarMeter`           | Handles segment mapping and direction logic |
| `SBK_BarDrive`           | Wrapper that adds animation support         |
| `SBK_BarMeterAnimations` | Provides animation control interface        |
| `SBK_MAX72xx`            | Software SPI driver for MAX7219/MAX7221     |
| `SBK_HT16K33`            | I2C driver for HT16K33 8x16 LED matrices    |

---

## 🪪 License

### Code

Licensed under the **MIT License**.

### Documentation

Licensed under **Creative Commons Attribution 4.0 (CC BY 4.0)**.

You are free to share and adapt the material, provided you give appropriate credit.

---

## 🧠 Credits

Library by **Samuel Barabé** (Smart Builds & Kits).

* MAX7219 driver inspired by [Eberhard Fahle](https://github.com/wayoda/LedControl)
* HT16K33 base adapted from [MikeS11's ProtonPack](https://github.com/MikeS11/ProtonPack)

---

## 🛠️ Support

* GitHub: [https://github.com/sbarabe/SBK-BarDrive](https://github.com/sbarabe/SBK-BarDrive)
* PCB files: [SBK_PCBs](https://github.com/sbarabe/SBK_PCBs)
* Contact: [smartbuildskits@gmail.com](mailto:smartbuildskits@gmail.com)

---
