/**
 * @file customMappingDemo.ino
 * @brief Demonstrates using SBK_BarDrive with a custom row/column mapping.
 *
 * Requirements:
 *      - Supported driver with complatible library (SBK_MAX72xx or SBK_HT16K33 libraries)
 *      - Bar meter display or leds array wired to driver
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @version 2.0.2
 * @license MIT
 */

#include <Arduino.h>

// ──────────────────────────────────────────────
// SBK BarDrive Configuration Flags
// ──────────────────────────────────────────────
#define SBK_BARDRIVE_WITH_ANIM

// ──────────────────────────────────────────────
// Bar Meter Custom Mapping Construction
// ──────────────────────────────────────────────
// Custom mapping format: {device, row, col} = {device index, anode, cathode}
//
// This defines the physical LED wiring for each logical segment in the bar meter.
//
// Each segment corresponds to one LED connected between:
//   - a driver’s row pin (anode, V+)
//   - a driver’s column pin (cathode, GND)
//
// Supported drivers (SBK-compatible):
//   - HT16K33:  rows = Rn (anode), cols = Cn (cathode)
//   - MAX72xx: rows = SEGn (anode), cols = DIGn (cathode)
//
// ✅ Mapping is in top-to-bottom order within each column,
//    progressing left-to-right across the display.
//
// ✅ You can freely assign segments to multiple devices
//    by varying the `device` index (first field).
//    This is useful when your LED matrix spans across multiple HT16K33 or MAX72xx ICs.
//
// Example layout shown below is a 4-row × 7-column matrix
// fully driven by device 0, equivalent to MatrixPreset::BL28_3005SK.
//
// ┌─────────────── Logical Mapping ───────────────┐
// │ Column →  0    1    2    3    4    5    6     │
// │ Row ↓                                          │
// │        [0,0,0][0,0,1][0,0,2]... ← top row      │
// │        [0,1,0][0,1,1][0,1,2]...                │
// │        [0,2,0][0,2,1][0,2,2]...                │
// │        [0,3,0][0,3,1][0,3,2]... ← bottom row   │
// └───────────────────────────────────────────────┘
//
// 💡 To split across devices, just change the `device` index in each entry.
//    For example:
//      {0, 0, 0}, {0, 1, 0}, ..., {1, 0, 4}, {1, 1, 4}, ...
//    This would place the first few columns on device 0,
//    and the next ones on device 1.
//
// IMPORTANT: Ensure your bar meter segments are wired accordingly for this mapping to work.
//
// This example defines a 4-row × 7-column matrix on device 0 — equivalent to using MatrixPreset::BL28_3005SK.
//
// {dev, row, col} = {dev, anode, cathode} layout, top-to-bottom, then left-to-right
const uint8_t customMapping[28][3] = {
    {0, 0, 0}, {0, 1, 0}, {0, 2, 0}, {0, 3, 0}, // Column 0
    {0, 0, 1}, {0, 1, 1}, {0, 2, 1}, {0, 3, 1}, // Column 1
    {0, 0, 2}, {0, 1, 2}, {0, 2, 2}, {0, 3, 2}, // Column 2
    {0, 0, 3}, {0, 1, 3}, {0, 2, 3}, {0, 3, 3}, // Column 3
    {0, 0, 4}, {0, 1, 4}, {0, 2, 4}, {0, 3, 4}, // Column 4
    {0, 0, 5}, {0, 1, 5}, {0, 2, 5}, {0, 3, 5}, // Column 5
    {0, 0, 6}, {0, 1, 6}, {0, 2, 6}, {0, 3, 6} // Column 6
};

// ──────────────────────────────────────────────
// SELECT YOUR DRIVER SETUP
// Uncomment one of the following driver configurations
// ──────────────────────────────────────────────

/* === [A] Using MAX7219/MAX7221 via SOFTWARE SPI (any 3 digital pins) === */
// #define DIN_PIN A4 // Define software SPI Data In pin
// #define CLK_PIN A5 // Define software SPI Clock pin
// #define CS_PIN A3  // Define SPI Chip Select pin
// #include <SBK_MAX72xxSoft.h>
// SBK_MAX72xxSoft driver(DIN_PIN, CLK_PIN, CS_PIN, 1); // Construct MAX72xx software SPI driver instance for 1 device : (DataIn pin, Clock pin, Chip Select pin, devices number)
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxSoft> bar(&driver, 0, customMapping);  // (driver, device index, custom mapping)

/* === [B] Using MAX7219/MAX7221 via HARDWARE SPI (dedicated MCU SPI pins) === */
// #define CS_PIN A3
// #include <SBK_MAX72xxHard.h>
// SBK_MAX72xxHard driver(CS_PIN, 1); // Construct MAX72xx hardware SPI driver instance for 1 device : (Chip Select pin, devices number)
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxHard> bar(&driver, 0, customMapping);  // (driver, device index, custom mapping)

/* === [C] Using HT16K33 via I2C === */
#include <SBK_HT16K33.h>
const uint8_t NUM_DEV = 1;       // Only one device : DEV0
const uint8_t DEV0_IDX = 0;      // Device DEV0 index
const uint8_t DEV0_ADD = 0x70;   // I2C Address (typically 0x70–0x77)
const uint8_t DEV0_NUM_ROWS = 8; // 20-SOP HT16K33 with only 8 rows, 24-SOP has 12 rows, 28-SOP has 16 rows
SBK_HT16K33 driver(NUM_DEV);
#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_HT16K33> bar(&driver, 0, customMapping); // (driver, device index, custom mapping)

void setup()
{

#ifdef SBK_HT16K33_IS_DEFINED
    // HT16K33 driver instance setup (demo uses a single device)
    driver.setAddress(DEV0_IDX, DEV0_ADD);         // Set I2C address for device 0
    driver.setDriverRows(DEV0_IDX, DEV0_NUM_ROWS); // Set number of active anode outputs (rows)
#endif

    driver.begin();
    driver.setBrightness(0, 10);

    bar.setDirection(BarDirection::FORWARD);

    bar.animations().animInit().bounceFillUpDur(700).loop();
}

void loop()
{
    bar.animations().update();
    bar.show();
}
