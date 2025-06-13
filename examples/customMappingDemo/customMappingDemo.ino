/**
 * @file customMappingDemo.ino
 * @brief Demonstrates using SBK_BarDrive with a custom row/column mapping.
 */

#include <Arduino.h>
#define SBK_BARDRIVE_WITH_ANIM
#include <SBK_BarDrive.h>

#define DATA_PIN A4
#define CLK_PIN A5
#define CS_PIN A3
#include <SBK_MAX72xxSoft.h>
SBK_MAX72xxSoft driver(DATA_PIN, CLK_PIN, CS_PIN);

// Custom mapping: {row, col} = {anode, cathode}
// Each segment maps to an LED connected between the IC's row (anode) and column (cathode) outputs.
//
// This wiring convention matches how most LED drivers operate:
//  - MAX72xx: DIGx (row/anode), SEGx (column/cathode)
//  - HT16K33: Rx (row/anode), Cx (column/cathode)
//
// IMPORTANT: Ensure your bar meter segments are wired accordingly for this mapping to work.
//
// Mapping is indexed top-to-bottom, then left-to-right, as visualized in layout.
// This example forms a 4-row × 3-column matrix with 12 segments.
const uint8_t customMapping[12][2] = {
    {0, 0}, {0, 1}, {0, 2}, {1, 2}, {1, 1}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {3, 2}, {3, 1}, {3, 0}};

// Construct using custom mapping
SBK_BarDrive<SBK_MAX72xxSoft> bar(&driver, 0, customMapping); // (driver, device index, custom mapping)

void setup()
{

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
