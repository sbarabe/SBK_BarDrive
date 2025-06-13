/**
 * @file makingYourOwnAnim.ino
 * Example showing how to manually fill a bar meter based on analog input without built-in animations.
 *
 * This example uses SBK_BarDrive and SBK_MAX72xxSoft to display a live analog signal level.
 * The fill level is updated manually using setPixel() and show(), without calling any animations().
 * No delay() is used — non-blocking loop via millis() ensures responsiveness.
 *
 * Requirements:
 * - SBK_BarDrive and SBK_MAX72xxSoft
 * - A supported MAX72xx bar meter
 * - A live analog signal connected to A0
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @version 1.0.0
 * @license MIT
 */

#include <Arduino.h>
#include <SBK_BarDrive.h>

#define DATA_PIN 4     ///< Software SPI data pin
#define CLK_PIN 3      ///< Software SPI clock pin
#define CS_PIN 2       ///< Software SPI chip select pin
#define NUM_DEVICES 1  ///< Number of MAX72xx chips
#define ANALOG_PIN A0  ///< Analog input pin
#define UPDATE_INTV 50 ///< Refresh interval in milliseconds

#include <SBK_MAX72xxSoft.h>
SBK_MAX72xxSoft driver(DATA_PIN, CLK_PIN, CS_PIN, NUM_DEVICES);
SBK_BarDrive<SBK_MAX72xxSoft> bar(&driver, 0, BarMeterType::BL28_3005SK);

void setup()
{
    driver.begin();
    bar.setDirection(FORWARD); // Optional: use REVERSE for opposite orientation
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
        uint8_t segs = map(rawValue, 0, 1023, 0, bar.getSegNum());

        // Update bar manually
        for (uint8_t i = 0; i < bar.getSegNum(); i++)
        {
            bar.setPixel(i, i < segs);
        }

        bar.show(); // Push changes to the display
    }
}
