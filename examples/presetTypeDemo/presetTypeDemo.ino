/**
 * @file presetTypeDemo.ino
 * @brief Demonstrates using SBK_BarDrive with a preset BarMeterType mapping.
 */

#include <Arduino.h>
#define SBK_BARDRIVE_WITH_ANIM
#include <SBK_BarDrive.h>

#define DATA_PIN A4
#define CLK_PIN A5
#define CS_PIN A3
#include <SBK_MAX72xxSoft.h>
SBK_MAX72xxSoft driver(DATA_PIN, CLK_PIN, CS_PIN);

// Preset BarMeterType values available:
// - DEFAULT_BarMeter,  : Generic fallback layout (8×8 grid).
// - SBK_BarMeter_SK28, : Alias for **SBK BarMeter SK28** PCB using BL28_3005SK BarMeterType.
// - SBK_BarMeter_SA28, : Alias for **SBK BarMeter SA28** PCB using BL28_3005SA BarMeterType.
// - SBK_BarMeter_24,   : Alias for **SBK BarMeter 24** PCB 3×8 layout for 24 segments bar.
// - SBK_BarMeter_40,   : Alias for **SBK BarMeter 40** PCB 5×8 layout for 40 segments bar.
// - BL28_3005SK,       : Native mapping for BL28(Z)-3005SK bar meter (7×4 layout, common cathode).
// - BL28_3005SA,       : Native mapping for BL28(Z)-3005SA bar meter (4×7 layout, common anode).
// - CUSTOM (used internally for user-defined mappings)

// Construct using preset bar meter type mapping
SBK_BarDrive<SBK_MAX72xxSoft> bar(&driver, 0, BarMeterType::BL28_3005SK); // (driver, device index, BarMeterType)

void setup()
{
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