/**
 * @file genericMatrixDemo.ino
 * @brief Demonstrates a generic rows-by-columns matrix bar meter.
 * @version 2.1.1
 * @license MIT
 */

#include <Arduino.h>

#define SBK_BARDRIVE_WITH_ANIM

#include <SBK_MAX72xxSoft.h>
#include <SBK_BarDrive.h>

constexpr uint8_t DIN_PIN = A4;
constexpr uint8_t CLK_PIN = A5;
constexpr uint8_t CS_PIN = A3;

SBK_MAX72xxSoft driver(DIN_PIN, CLK_PIN, CS_PIN, 1);
SBK_BarDrive<SBK_MAX72xxSoft> bar(
    &driver, 0, 4, 7, BarDirection::FORWARD);

void setup()
{
    Serial.begin(115200);
    driver.begin();

    // A generic 4 x 7 matrix contains 28 logical segments.
    Serial.print(F("Generic matrix segments: "));
    Serial.println(bar.getSegsNum());

    bar.animations().animInit().bounceFillUpIntv(25).loop();
}

void loop()
{
    bar.animations().update();
    bar.show();
}
