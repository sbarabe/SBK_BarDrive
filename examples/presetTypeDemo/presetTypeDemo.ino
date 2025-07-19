/**
 * @file presetTypeDemo.ino
 * @brief Demonstrates using SBK_BarDrive with a preset BarMeterType mapping.
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
#define SBK_BARDRIVE_WITH_ANIM // Give access to preset animations and controls.

// ──────────────────────────────────────────────
// SBK BarDrive MatrixPreset info
// ──────────────────────────────────────────────
/**
 * More preset could be add upon time or on request.
 * Preset MatrixPreset values currtently available:
 *      - SBK_BarMeter_SK28 : Alias for **SBK BarMeter SK28** PCB using BL28_3005SK BarMeterType.
 *      - SBK_BarMeter_SA28 : Alias for **SBK BarMeter SA28** PCB using BL28_3005SA BarMeterType.
 *      - BL28_3005SK       : Native mapping for BL28-3005SK bar meter (7×4 layout, common cathode).
 *      - BL28_3005SA       : Native mapping for BL28-3005SA bar meter (4×7 layout, common anode).
 */


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
// SBK_BarDrive<SBK_MAX72xxSoft> bar(&driver, 0, MatrixPreset::BL28_3005SK); ///< Construct using matrix type bar meter preset (auto-mapped layout) : (driver, device index, MatrixPreset type)

/* === [B] Using MAX7219/MAX7221 via HARDWARE SPI (dedicated MCU SPI pins) === */
// #define CS_PIN A3 ///< Define SPI Chip Select pin
// #include <SBK_MAX72xxHard.h>
// SBK_MAX72xxHard driver(CS_PIN, 1); ///< Construct MAX72xx hardware SPI driver instance for 1 device : (Chip Select pin, devices number)
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxHard> bar(&driver, 0, MatrixPreset::BL28_3005SK); ///< Construct using matrix type bar meter preset (auto-mapped layout) : (driver, device index, MatrixPreset type)

/* === [C] Using HT16K33 via I2C === */
#include <SBK_HT16K33.h>
const uint8_t NUM_DEV = 1;       ///< Only one device : DEV0
const uint8_t DEV0_IDX = 0;      ///< Device DEV0 index
const uint8_t DEV0_ADD = 0x70;   ///< I2C Address (typically 0x70–0x77)
const uint8_t DEV0_NUM_ROWS = 8; ///< 20-SOP HT16K33 with only 8 rows, 24-SOP has 12 rows, 28-SOP has 16 rows
SBK_HT16K33 driver(NUM_DEV);
#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_HT16K33> bar(&driver, 0, MatrixPreset::BL28_3005SK); ///< Construct using matrix type bar meter preset (auto-mapped layout) : (driver, device index, MatrixPreset type)

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

    bar.animations().animInit().fillUpIntv(50).loop();
}

void loop()
{
    bar.animations().update();
    bar.show();
}