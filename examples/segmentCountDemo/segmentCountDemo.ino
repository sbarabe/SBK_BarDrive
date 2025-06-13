/**
 * @file segmentCountDemo.ino
 * @brief Demonstrates using SBK_BarDrive with a segment count (auto-mapped).
 */

#include <Arduino.h>
#define SBK_BARDRIVE_WITH_ANIM
#include <SBK_BarDrive.h>

#define DATA_PIN A4
#define CLK_PIN A5
#define CS_PIN A3
#include <SBK_MAX72xxSoft.h>
SBK_MAX72xxSoft driver(DATA_PIN, CLK_PIN, CS_PIN);

// Construct using segment count (auto-mapped layout)
SBK_BarDrive<SBK_MAX72xxSoft> bar(&driver, 0, 24); // (driver, device index, number of bar segments)

/*
 * Segment mapping explanation:
 *
 * When using a segment count constructor (e.g., 24), the library auto-generates a
 * row-column mapping assuming a default 8×8 LED matrix layout.
 *
 * Segments are assigned left to right, top to bottom:
 *  - Segment  0 → [0,0]
 *  - Segment  1 → [0,1]
 *  - ...
 *  - Segment  7 → [0,7]
 *  - Segment  8 → [1,0]
 *  - ...
 *  - Segment 23 → [2,7]
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
    driver.begin();
    driver.setBrightness(0, 10);

    bar.setDirection(BarDirection::FORWARD);

    bar.animations().animInit().scrollingUpBlocks(60, 2, 1).loop();
}

void loop()
{
    bar.animations().update();
    bar.show();
}