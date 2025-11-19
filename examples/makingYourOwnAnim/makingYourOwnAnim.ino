/**
 * @file makingYourOwnAnim.ino
 * @brief Example showing how to manually fill a bar meter based on analog input WITHOUT built-in animations.
 *
 * This example show how you can construct your own animations.
 * The animation built here displays a live analog signal level.
 * The fill level is updated manually using setPixel() and show(), without calling preset animations().
 * No delay() is used — a non-blocking loop based on millis() ensures responsiveness.
 *
 * Requirements:
 *      - Supported driver with complatible library (SBK_MAX72xx or SBK_HT16K33 libraries)
 *      - Bar meter display or leds array wired to driver
 *      - A live analog signal connected to A0
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @version 2.0.4
 * @license MIT
 */

#include <Arduino.h>

#define ANALOG_PIN A0         ///< Analog input pin
#define UPDATE_INTV 50        ///< Refresh interval in milliseconds
unsigned long lastUpdate = 0; ///< animations update tracking

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
    bar.setDirection(BarDirection::FORWARD); // Optional: use REVERSE for opposite orientation
}

/**
 * @brief Main update loop using non-blocking timing.
 */
void loop()
{
    uint32_t now = millis();
    if (now - lastUpdate >= UPDATE_INTV)
    {
        lastUpdate = now;

        // Read and map signal
        uint16_t rawValue = analogRead(ANALOG_PIN);
        uint8_t segs = map(rawValue, 0, 1023, 0, bar.getSegsNum());

        // Update bar manually
        for (uint8_t i = 0; i < bar.getSegsNum(); i++)
        {
            bar.setPixel(i, i < segs);
        }

        bar.show(); // Push changes to the display
    }
}
