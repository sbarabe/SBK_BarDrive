/**
 * @example splitDriverDevicesBarMeter.ino
 * @brief Example showing how to split a bar meter on multiple driver devices.
 *
 * Requirements:
 *      - Supported driver with complatible library (SBK_MAX72xx or SBK_HT16K33 libraries)
 *      - Bar meter display or leds array wired to driver
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @version 2.0.3
 * @license MIT
 */

#include <Arduino.h>

// ──────────────────────────────────────────────
// SBK BarDrive Configuration Flags
// ──────────────────────────────────────────────
#define SBK_BARDRIVE_WITH_ANIM // Give access to preset animations and controls.

// ──────────────────────────────────────────────
// SELECT YOUR DRIVER SETUP
// Uncomment one of the following driver configurations
// ──────────────────────────────────────────────

/* === [A] Using MAX7219/MAX7221 via SOFTWARE SPI (any 3 digital pins) === */
// #define DIN_PIN A4 ///< Define software SPI Data In pin
// #define CLK_PIN A5 ///< Define software SPI Clock pin
// #define CS_PIN A3  ///< Define SPI Chip Select pin
// #include <SBK_MAX72xxSoft.h>
// SBK_MAX72xxSoft driver(DIN_PIN, CLK_PIN, CS_PIN, 2); ///< Construct MAX72xx software SPI driver instance for 2 devices : (DataIn pin, Clock pin, Chip Select pin, devices number)
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxSoft> bar1(&driver, 0, 8, BarDirection::FORWARD, 0); ///< Construct using segments count starting at segment 0 (auto-mapped layout) : (driver, device index, segments count, BarDirection, segment offset)
// SBK_BarDrive<SBK_MAX72xxSoft> bar2(&driver, 0, 8, BarDirection::FORWARD, 8); ///< Construct using segments count starting at segment 0 (auto-mapped layout) : (driver, device index, segments count, BarDirection, segment offset)
// SBK_BarDrive<SBK_MAX72xxSoft> bar3(&driver, 0, MatrixPreset::BL28_3005SK, BarDirection::FORWARD, 2); ///< Construct using matrix type bar meter preset (auto-mapped layout) : (driver, device index, MatrixPreset type, BarDirection, row offset)

/* === [B] Using MAX7219/MAX7221 via HARDWARE SPI (dedicated MCU SPI pins) === */
// #define CS_PIN A3 ///< Define SPI Chip Select pin
// #include <SBK_MAX72xxHard.h>
// SBK_MAX72xxHard driver(CS_PIN, 2); ///< Construct MAX72xx hardware SPI driver instance for 2 devices : (Chip Select pin, devices number)
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxHard> bar1(&driver, 0, 8, BarDirection::FORWARD, 0); ///< Construct using segments count starting at segment 0 (auto-mapped layout) : (driver, device index, segments count, BarDirection, segment offset)
// SBK_BarDrive<SBK_MAX72xxHard> bar2(&driver, 0, 8, BarDirection::FORWARD, 8); ///< Construct using segments count starting at segment 0 (auto-mapped layout) : (driver, device index, segments count, BarDirection, segment offset)
// SBK_BarDrive<SBK_MAX72xxHard> bar3(&driver, 0, MatrixPreset::BL28_3005SK, BarDirection::FORWARD, 2); ///< Construct using matrix type bar meter preset (auto-mapped layout) : (driver, device index, MatrixPreset type, BarDirection, row offset)

/* === [C] Using HT16K33 via I2C === */
#include <SBK_HT16K33.h>
const uint8_t NUM_DEV = 2;       ///< Number of HT16K33 devices in use
const uint8_t DEV0_IDX = 0;      ///< Device 0 index
const uint8_t DEV1_IDX = 1;      ///< Device 1 index
const uint8_t DEV0_ADD = 0x70;   ///< I2C address for device 0 (typically 0x70–0x77)
const uint8_t DEV1_ADD = 0x71;   ///< I2C address for device 1 (typically 0x70–0x77)
const uint8_t DEV0_NUM_ROWS = 8; ///< Device 0: 20-SOP = 8 rows, 24-SOP = 12 rows, 28-SOP = 16 rows
const uint8_t DEV1_NUM_ROWS = 8; ///< Device 1: 20-SOP = 8 rows, 24-SOP = 12 rows, 28-SOP = 16 rows
SBK_HT16K33 driver(NUM_DEV);
#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_HT16K33> bar1(&driver, 0, 24, BarDirection::FORWARD, 0);                        ///< 24-segment bar on device 0, starting at segment 0
SBK_BarDrive<SBK_HT16K33> bar2(&driver, 0, 24, BarDirection::FORWARD, 24);                       ///< 24-segment bar on device 0, starting at segment 24 (offset)
SBK_BarDrive<SBK_HT16K33> bar3(&driver, 0, MatrixPreset::BL28_3005SK, BarDirection::FORWARD, 6); ///< BL28-3005SK preset, starts at row offset 6

/**
/**
 * @brief In this example, three bar meters are created using SBK_BarDrive across 2 devices with 8 rows (anodes) outputs :
 *
 * - bar1: A 24-segment 1D bar meter on device 0 (segments 0–23),
 *         mapped from row 0 to row 2, across columns 0–7.
 *
 * - bar2: Another 24-segment 1D bar meter on device 0 (segments 24–47),
 *         mapped from row 3 to row 5, across columns 0–7.
 *
 * - bar3: A 28-segment 2D matrix bar meter using MatrixPreset::BL28_3005SK
 *         (4 rows × 7 columns), starting at row offset 6.
 *         This fills:
 *             - Device 0: rows 6–7, columns 0–6
 *             - Device 1: rows 0–1, columns 0–6
 */

void setup()
{
    Serial.begin(115200);

#ifdef SBK_HT16K33_IS_DEFINED
    // HT16K33 driver instance setup (demo uses a single device)
    driver.setAddress(DEV0_IDX, DEV0_ADD);         // Set I2C address for device 0
    driver.setDriverRows(DEV0_IDX, DEV0_NUM_ROWS); // Set number of active anode outputs (rows)
    driver.setAddress(DEV1_IDX, DEV1_ADD);         // Set I2C address for device 0
    driver.setDriverRows(DEV1_IDX, DEV1_NUM_ROWS); // Set number of active anode outputs (rows)
#endif

    driver.begin();
    driver.setBrightness(0, 10);

    // Outputs mapping to serial
    bar1.debugSegmentMapping();
    Serial.println(), Serial.println();
    bar2.debugSegmentMapping();
    Serial.println(), Serial.println();
    bar3.debugSegmentMapping();

    bar1.animations().animInit().fillUpIntv(25).loop();
    bar2.animations().animInit().bounceFillUpIntv(25).loop();
    bar3.animations().animInit().beatPulse(116);
}

void loop()
{
    bar1.animations().update();
    bar2.animations().update();
    bar3.animations().update();

    driver.show();
}