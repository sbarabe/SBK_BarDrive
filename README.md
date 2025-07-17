# SBK\_BarDrive Library

High-level Arduino library for controlling animated LED bar meters using MAX7219/MAX7221 or HT16K33 drivers. Ideal for prop-making, visual meters, and signal-driven lighting effects.

---

## 🆕 What's New in v2.0.0

Version 2.0.0 introduces major architectural and feature improvements. Previous versions (1.x) are deprecated due to internal changes in how offset positioning and multi-device spanning are handled.
🔧 Core Enhancements

    * Block-based animations — upwardUnstackingBlocks() and downwardUnstackingBlocks()

    * Signal-driven modes — followSignalFloatingPeak()

    * Offset support — bar meters can now start from arbitrary row/column or segment positions

    * Multi-device mapping — segments can span across multiple driver devices (via auto-mapping, matrix presets or custom mappings)

    * Full custom segment mapping — define precise {device, row, col} mappings for advanced layouts

⚠️ Deprecation Notice

Previous versions (1.x) are no longer compatible due to architectural changes. Please migrate existing projects by updating segment mappings and constructor parameters to match the v2.0+ format.

## ✨ Features

* Unified API for both **SPI** (MAX72xx) and **I2C** (HT16K33) LED drivers
* Built-in **bar meter animations** (e.g., filling, bouncing, signal-following, block effects)
* Support for **custom \[row, col] segment mappings** or preset types
* Compile-time options to optimize for memory:

  * `SBK_BARDRIVE_WITH_ANIM` to include animations
  * `SBK_TRACK_PIXEL_STATE` to enable pixel state caching
* Designed for **SBK BarMeter** and **SBK BarDrive** PCBs
* Reverse display modes and flexible mapping
* Internal buffer with batch `.show()` updates
* **Software SPI (MAX72xx)** — works on any 3 digital pins: `DATA`, `CLK`, and `CS`
* **I2C (HT16K33)** — uses `SDA` and `SCL` pins (standard I2C bus)

---

## ⚙️ Supported Hardware Combinations

This library is compatible with **any LED matrix or bar display** using `MAX7219`, `MAX7221`, or `HT16K33` drivers — as long as a valid `[row, col]`, aka `[anode,cathode]`, segment mapping is provided or configured using a built-in preset.

When using custom segment mappings with SBK_BarDrive, each segment is defined as a {row, col} pair — representing the physical LED connection:

{row, col} = {anode, cathode}

This matches the wiring convention of common LED driver ICs:
| Driver  | Row (Anode / V+)    | Column (Cathode / GND) |
| ------- | ------------------- | ---------------------- |
| MAX72xx | `SEGx` (source, V+) | `DIGx` (sink, GND)     |
| HT16K33 | `Rx`  (source, V+)  | `Cx`   (sink, GND)     |


This library is designed to work with the following SBK hardware combinations:

* 🟢 **SBK BarDrive *SK*28 PCB** + **SBK BarMeter 28 PCB**  + **BL28-3005SKxx type (28seg *com. cathode* Bar Meter)**
* 🔵 **SBK BarDrive *SA*28 PCB** + **SBK BarMeter 28 PCB**  + **BL28-3005SAxx type (28seg *com. anode* Bar Meter)**
* 🟣 **SBK BarDrive 64 PCB**   + **SBK BarMeter 24 PCB**  + **3x B8x type (8seg com. cathode Bar Meter)**
* 🟠 **SBK BarDrive 64 PCB**   + **SBK BarMeter 40 PCB**  + **5x B8x type (8seg com. cathode Bar Meter)**

For more information, schematics, and PCB files, visit: [https://github.com/sbarabe/SBK_PCBs](https://github.com/sbarabe/SBK_PCBs)

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
   #include <SBK_BarDrive.h>
   ```

---

## 🔊 Quick Start Examples

### Using MAX7219:

```cpp
#define SBK_BARDRIVE_WITH_ANIM

#include <SBK_MAX72xx.h>
SBK_MAX72xx max72xx(DATA_PIN, CLK_PIN, CS_PIN, 1);

#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_MAX72xx> bar(&max72xx, 0, BarMeterType::BL28_3005SK);

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
    ht..setDriverRows(0,8); // Set HT16K33 device rows (anodes) outputs number : 20-SOP = 8, 24-SOP = 12, 28-SOP = 16
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
#include <SBK_MAX72xx.h>
SBK_MAX72xx max72xx(DATA_PIN, CLK_PIN, CS_PIN, 2); // 2 devices

#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_MAX72xx> bar(&max72xx, 0, mapping);

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
```cpp
bar.animations().pause();
bar.animations().resume();
bar.animations().stop();
bar.animations().loop();
bar.animations().noLoop();
bar.animations().toggleLogic();
bar.animations().invertLogic();
bar.animations().resetLogic();
````

All helper functions return a reference to the `SBK_BarMeterAnimations` object, allowing chainable expressions like:

````cpp
bar.animations().pause().setLogic(true).resume().stopBlockEmission();
```cpp
bar.animations()
    .pause()
    .setLogic(true)
    .resume()
    .stopBlockEmission();
````

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

> ✏️ Feel free to customize any URLs or email addresses before publishing!
