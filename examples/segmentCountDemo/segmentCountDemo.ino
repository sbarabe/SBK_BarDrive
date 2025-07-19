/**
 * @file segmentCountDemo.ino
 * @brief Demonstrates SBK_BarDrive using only segment count for 1D bar meter type (auto-mapped).
 * 
 * When the bar meter is not based on a 2D matrix layout, the SBK_BarDrive instance
 * can be created using only the segment count — as long as the physical wiring
 * follows the expected linear (sequential) order. 
 * 
 * ⚠️ Important: This approach assumes that the LEDs are wired sequentially from
 * the driver's output pins, following the native segment order:
 *
 * - For MAX72xx: SEG0/COL0 → SEG0/COL1 → ... → SEG70/COL7, then SEG1/COL0, etc.
 * - For HT16K33: ROW0/COL0 → ROW0/COL1 → ... → ROW0/COL7, then ROW1/COL0, etc.
 *
 * Ensure your bar meter is wired to match this order for correct auto-mapping behavior.
 * 
 * Requirements:
 *      - Supported driver with complatible library (SBK_MAX72xx or SBK_HT16K33 libraries)
 *      - Bar meter display or leds array wired to driver
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @version 2.0.1
 * @license MIT
 */

#include <Arduino.h>

// ──────────────────────────────────────────────
// SBK BarDrive Configuration Flags
// ──────────────────────────────────────────────
#define SBK_BARDRIVE_WITH_ANIM ///< Give access to preset animations and controls.

// ──────────────────────────────────────────────
// SELECT YOUR DRIVER SETUP
// Uncomment one of the following driver configurations
// ──────────────────────────────────────────────

/* === [A] Using MAX7219/MAX7221 via SOFTWARE SPI (any 3 digital pins) === */
// #define DIN_PIN A4 ///< Define software SPI Data In pin
// #define CLK_PIN A5 ///< Define software SPI Clock pin
// #define CS_PIN A3  ///< Define SPI Chip Select pin
// #include <SBK_MAX72xxSoft.h>
// SBK_MAX72xxSoft driver(DIN_PIN, CLK_PIN, CS_PIN, 1); ///< Construct MAX72xx software SPI driver instance for 1 device : (DataIn pin, Clock pin, Chip Select pin, devices number)
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxSoft> bar(&driver, 0, 24); ///< Construct using segment count (auto-mapped layout) : (driver, device index, number of bar segments)

/* === [B] Using MAX7219/MAX7221 via HARDWARE SPI (dedicated MCU SPI pins) === */
// #define CS_PIN A3 ///< Define SPI Chip Select pin
// #include <SBK_MAX72xxHard.h>
// SBK_MAX72xxHard driver(CS_PIN, 1); ///< Construct MAX72xx hardware SPI driver instance for 1 device : (Chip Select pin, devices number)
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxHard> bar(&driver, 0, 24); ///< Construct using segment count (auto-mapped layout) : (driver, device index, number of bar segments)

/* === [C] Using HT16K33 via I2C === */
#include <SBK_HT16K33.h>
const uint8_t NUM_DEV = 1;       ///< Only one device : DEV0
const uint8_t DEV0_IDX = 0;      ///< Device DEV0 index
const uint8_t DEV0_ADD = 0x70;   ///< I2C Address (typically 0x70–0x77)
const uint8_t DEV0_NUM_ROWS = 8; ///< 20-SOP HT16K33 with only 8 rows, 24-SOP has 12 rows, 28-SOP has 16 rows
SBK_HT16K33 driver(NUM_DEV);
#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_HT16K33> bar(&driver, 0, 24); ///< Construct using segment count (auto-mapped layout) : (driver, device index, number of bar segments)

/*
 * Segment mapping explanation:
 *
 * When using a segment count constructor (e.g., 24), the library auto-generates a
 * row-column mapping assuming a default 8×8 LED matrix layout.
 *
 * Segments are assigned left to right, top to bottom:
 *  - Segment index  0 → [0,0]
 *  - Segment index  1 → [0,1]
 *  - ...
 *  - Segment index  7 → [0,7]
 *  - Segment index  8 → [1,0]
 *  - Segment index  9 → [1,1]
 *  - Segment index 10 → [1,2]
 *  - ...
 *  - Segment index 23 → [2,7]
 *
 * This layout is convenient for generic displays or matrix testing,
 * but may not your actual bar meter wiring with the IC chip driver.
 * For this layout to work, you must wire the bar meter display to
 * the IC driver chip filling the row in order.
 *
 * Examaple : to match this mapping with 3× B08 bar meter displays (3×8 = 24 segments)
 * using a MAX7219/MAX7221 IC, you must wire each display such that **columns fill first**:
 *
 * First B08 bar meter (SEG 0–7 wired to DIG_0 aka row 0)):
 *   SEG 0   ---SEG_DP---|>|--- DIG_0
 *   SEG 1   ---SEG_A----|>|--- DIG_0
 *   SEG 2   ---SEG_B----|>|--- DIG_0
 *   ...     ...
 *   SEG 7   ---SEG_G----|>|--- DIG_0
 *
 * Second B08 bar meter (SEG 8–15 wired to DIG_1 aka row 1 ):
 *   SEG 8   ---SEG_DP---|>|--- DIG_1
 *   ...
 *   SEG 15  ---SEG_G----|>|--- DIG_1
 *
 * Third B08 bar meter (SEG 16–23 wired to DIG_2 aka row 2):
 *   SEG 16  ---SEG_DP---|>|--- DIG_2
 *   ...
 *   SEG 23  ---SEG_G----|>|--- DIG_2
 *
 * ✅ Use a preset BarMeterType or custom mapping array if your wiring does not
 * follow this column-first convention.
 */

void setup()
{
#ifdef SBK_HT16K33_IS_DEFINED
    // HT16K33 driver instance setup (demo uses a single device)
    driver.setAddress(DEV0_IDX, DEV0_ADD);         ///< Set I2C address for device 0
    driver.setDriverRows(DEV0_IDX, DEV0_NUM_ROWS); ///< Set number of active anode outputs (rows)
#endif

    driver.begin();
    driver.setBrightness(0, 10);

    bar.setDirection(BarDirection::FORWARD);

    bar.animations().animInit().fillUpIntv(25).loop();
}

void loop()
{
    bar.animations().update();
    bar.show();
}