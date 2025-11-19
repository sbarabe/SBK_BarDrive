
/**
 * @file animationShowcase.ino
 * @brief Demonstrates various SBK_BarDrive animations with signal simulation and automatic cycling.
 *
 * This example runs multiple LED bar animations sequentially, including:
 * - Random fill/empty
 * - Fill up/down
 * - Scrolling blocks
 * - Signal-following animations
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
#include <math.h>

// ──────────────────────────────────────────────
// SBK BarDrive Configuration Flags
// ──────────────────────────────────────────────
#define SBK_BARDRIVE_WITH_ANIM // Give access to preset animations and controls.

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
// SBK_BarDrive<SBK_MAX72xxSoft> bar(&driver, 0, MatrixPreset::BL28_3005SK); // Construct using matrix type bar meter preset (auto-mapped layout) : (driver, device index, MatrixPreset type)

/* === [B] Using MAX7219/MAX7221 via HARDWARE SPI (dedicated MCU SPI pins) === */
// #define CS_PIN A3
// #include <SBK_MAX72xxHard.h>
// SBK_MAX72xxHard driver(CS_PIN, 1); // Construct MAX72xx hardware SPI driver instance for 1 device : (Chip Select pin, devices number)
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxHard> bar(&driver, 0, MatrixPreset::BL28_3005SK); // Construct using matrix type bar meter preset (auto-mapped layout) : (driver, device index, MatrixPreset type)

/* === [C] Using HT16K33 via I2C === */
#include <SBK_HT16K33.h>
const uint8_t NUM_DEV = 1;       // Only one device : DEV0
const uint8_t DEV0_IDX = 0;      // Device DEV0 index
const uint8_t DEV0_ADD = 0x70;   // I2C Address (typically 0x70–0x77)
const uint8_t DEV0_NUM_ROWS = 8; // 20-SOP HT16K33 with only 8 rows, 24-SOP has 12 rows, 28-SOP has 16 rows
SBK_HT16K33 driver(NUM_DEV);
#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_HT16K33> bar(&driver, 0, MatrixPreset::BL28_3005SK); // Construct using matrix type bar meter preset (auto-mapped layout) : (driver, device index, MatrixPreset type)

/*
 * 💡 Default example assumes a BL28_3005SK 28-segment matrix type bar meter.
 * You may change the MatrixPreset or constructor method to match your specific hardware.
 * See README and other examples for linear 1D (non matrix) type or custom mapping options.
 */

uint8_t demoMode = 0;
unsigned long lastSwitch = 0;
const unsigned long switchInterval = 10000;
bool lastIsRunning = false;
bool setupFlag = false;
bool update = false;
bool lastUpdate = false;

uint16_t fakeSignal1 = 0;
uint16_t fakeSignal2 = 0;
uint16_t generateFakeSignal(uint32_t tMillis);

void setup()
{
    Serial.begin(115200);

    Serial.print(F("Animations showcase setup..."));

#ifdef SBK_HT16K33_IS_DEFINED
    // HT16K33 driver instance setup (demo uses a single device)
    driver.setAddress(DEV0_IDX, DEV0_ADD);         // Set I2C address for device 0
    driver.setDriverRows(DEV0_IDX, DEV0_NUM_ROWS); // Set number of active anode outputs (rows)
#endif
    driver.begin();              // Initialize the driver
    driver.setBrightness(0, 10); // Set brightness level (0 = dim, 15 = bright)

    bar.setDirection(BarDirection::FORWARD); // Set initial bar fill direction

    demoMode = 12; // Setup demoMode index to show demoMode 0 at fisrt itération;
    Serial.println(F("done! Showcase begin!"));
}

/*
 * 🔄 Direction vs. Logic
 *
 * SBK_BarDrive separates animation logic from display direction.
 *
 * - Logic: Defines how the animation progresses internally.
 *          Examples: fill vs. empty, emit vs. absorb, colliding vs. exploding.
 *          Changing the logic affects how segments are activated over time.
 *
 * - Direction: Controls how the animation is rendered (mirrored or not).
 *              FORWARD = segments 0 → N−1
 *              REVERSE = segments N−1 → 0
 *              This affects visual orientation only — not the underlying logic.
 *
 * Examples:
 *   - fillUpIntv() always fills from the first to the last segment logically.
 *   - bar.setDirection(REVERSE) mirrors that fill so it appears to fill from top to bottom.
 *   - Inverting fillUpIntv() will result in emptying fromtop to bottom (emptyDown),
 *     not a top-down fill. The logic changed, not the direction.
 *
 * 🛑 Not all animations support inverted logic:
 *   - Animations like scrolling blocks or bounce fill ignore logic inversion.
 *   - For these, only direction (setDirection, toggleDirection, resetDirection) will alter appearance.
 *
 * ✅ You can combine logic and direction to create symmetric or inverted animations
 * without modifying the animation source.
 */

void loop()
{

    /*
     * Update current animation.
     * IMPORTANT : animations().update() must be called every loop to render the current animation.
     */
    bar.animations().update();
    /*
     * Push animation into bar meter driver.
     * IMPORTANT : must be call every loop after animations updates.
     */
    bar.show();

    if (update != lastUpdate)
    {
        Serial.println(update);
        lastUpdate = update;
    }
    /*
     * If animation is not running, stop current animation,
     * reset animation logic, and increment demoMode to start next animation.
     *
     * IMPORTANT : animations().update() must be called every loop to render the current animation.
     */
    if (bar.animations().isRunning() == false) // update animation and check if it's done
    {
        lastSwitch = millis();
        bar.animations().stop().resetLogic(); // Clear previous

        if (setupFlag == false)
            setupFlag = true;
        else
            Serial.println(F("  Current animation is stopped!"));

        Serial.println();
        demoMode = (demoMode + 1) % 12; // Increment demoMode by 1 and reset it to 0 if greater then 11.
    }

    /*
     * Some selected animations for demonstration purposes.
     * Also demonstrate some possibles animations helpers.
     *
     * Check README file and Doxygen documentation for full animations and controls list.
     */
    switch (demoMode)
    {
    case 0:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().fillUpIntv(30).loop();
            Serial.println(F("Demo 0 : fillUp at intv and loop."));
        }
        // When logic is inverted, animation empty down instead of fill up.
        break;
    case 1:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().bounceFillUpIntv(20, 60, 20, 80).loop();
            Serial.println(F("Demo 1 : fillUp fast and emptyDown slow at intv with range limits and loop."));
        }
        // When logic is inverted nothing change : it's a non inverting logic animation.
        // A reverseDir() or toggleDir() could be use to change the animation direction to bounce down...
        break;
    case 2:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().collidingBlocks(25, 4, 4, 6).loop();
            Serial.println(F("Demo 2 : 6 blocks are emitted from edges and collide at center."));
            // 6 blocks (4 pixels length, 4 pixels space) are emitted at both edges and collide at the display center.
            // The blocks move at fixed interval (25ms), when all blocks are exited, animation loops...
        }
        // When logic is inverted, animation emits blocks from the center.
        break;
    case 3:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().explodingBlocks(25, 8, 10);
            Serial.println(F("Demo 3 : Infinite blocks are emitted from center."));
            // Infinite blocks (8 pixels length, 10 pixels space) are emitted from display center.
            // The blocks move at fixed interval (25ms), this animation is an ever running animation,
            // loop()/noLoop() have no effect since there is an infinite blocks number...
        }
        // When logic is inverted, animation emits blocks from the edges toward the center.
        break;
    case 4:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().scrollingUpBlocks(25, 5, 3, 8).loop();
            Serial.println(F("Demo 4 : 4 blocks scrolling from bottom to top."));
            // 4 blocks (5 pixels length, 3 pixels space) from bottom edge and scroll to top.
            // The blocks move at fixed interval (25ms), when all blocks are exited, animation loops...
        }
        // When logic is inverted nothing change : it's a non inverting logic animation.
        // A reverseDir() or toggleDir() could be use to change the animation direction tio scroll down block...
        break;
    case 5:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().scrollingDownBlocks(25, 2, 4);
            Serial.println(F("Demo 5 : Infinite blocks scrolling from top to bottom."));
            // Infinite blocks (2 pixels length, 4 pixels space) from top to bottom edge.
            // The blocks move at fixed interval (25ms), this animation is an ever running animation,
            // loop()/noLoop() have no effect since there is an infinite blocks number...
        }
        // When logic is inverted nothing change : it's a non inverting logic animation.
        // A reverseDir() or toggleDir() could be use to change the animation direction tio scroll down block...
        break;
    case 6:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().followSignalSmooth(&fakeSignal1, 100, 0, 1023, 70, 5);
            Serial.println(F("Demo 6 : Bar fill up to smoothed signal pointer."));
            // Visual update are made at fixed interval (100ms).
            // Display range is mapped from signal min/max values (min 0, max  1023
            // A 30% smooth factor is applied, sampling is made at 5ms.
            // loop()/noLoop() have no effect since it's an ever running animation
        }
        // When logic is inverted nothing change : it's a non inverting logic animation.
        // A reverseDir() or toggleDir() could be use to change the animation direction to fill down to signal level...
        break;
    case 7:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().followDualSignalFromCenter(&fakeSignal1, 100);
            Serial.println(F("Demo 7 : Bar fill from center toward edges at smoothed signal level."));
            // fakeSignal1 is mirror at center.
            // Visual update are made at fixed interval (100ms).
            // Display range is mapped from signal min/max default values (min 0, max  1023).
            // A 30% default smooth factor is applied, sampling is made at 5ms default value.
            // loop()/noLoop() have no effect since it's an ever running animation
        }
        // When logic is inverted, the signal level fill is made from the edges toward center.
        // A reverseDir() or toggleDir() would not affect this animation because it's already mirrored at the cent4er.
        break;
    case 8:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().followSignalFloatingPeak(&fakeSignal1);
            Serial.println(F("Demo 8 : Bar fill at smoothed signal level with floating peak pixel."));
        }
        // When logic is inverted nothing change : it's a non inverting logic animation.
        // A reverseDir() or toggleDir() could be use to change the animation direction pulse from top...

        break;
    case 9:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().downStackingBlocks(20, 1, 0).loop();
            Serial.println(F("Demo 9 : blocks fall from top and stack at the bottom"));
        }
        // When logic is inverted blocks are unstacking and flying out.
        break;
    case 10:
        if (!bar.animations().isRunning())
        {
            bar.animations().setAllOff().animInit().randomFill(50);
            Serial.println(F("Demo 10 : OFF pixels are turned ON in a random pattern at intv."));
        }
        break;
    case 11:
        if (!bar.animations().isRunning())
        {
            bar.animations().animInit().randomEmpty(50);
            Serial.println(F("Demo 11 : ON pixels are turned OFF in a random pattern at intv."));
        }
        break;
    }

    // Toggle logic at after 5 secondes trough animation
    if (bar.animations().isNonInvertingLogicAnim() == false && bar.animations().isLogicInverted() == false && millis() - lastSwitch >= 5000)
    {
        if ((demoMode == 3 || demoMode == 5 || demoMode == 7) || (bar.animations().animPendingLoop() == true))
        {
            bar.animations().invertLogic();
            Serial.println(F("  Animation logic inverted !"));
        }
    }

    // End this demoMode
    if (millis() - lastSwitch > 10000)
    {
        if (bar.animations().isLoopEnabled() == true)
        {
            bar.animations().noLoop();
            Serial.println(F("  Loop stopped !"));
        }
        if (demoMode == 6 || demoMode == 7 || demoMode == 8)
        {
            bar.animations().stop();
        }
        if ((demoMode == 3 || demoMode == 5) && (bar.animations().isBlockEmissionEnabled() == true))
        {
            bar.animations().stopBlockEmission();
            Serial.println(F("  Blocks emission stopped !"));
        }
    }

    /* Simulate a pair of sine-wave analog signals for demonstration purposes */
    fakeSignal1 = generateFakeSignal(millis());
    fakeSignal2 = generateFakeSignal(millis() + 2157);
}

uint16_t generateFakeSignal(uint32_t tMillis)
{
    // Simulates a noisy analog signal using a sine wave + random jitter

    float baseFreq = 0.0015; // Controls main wave speed
    float noiseFreq = 0.006; // Adds irregularity
    float noiseAmp = 300;    // Max +/- variation in amplitude

    float base = sin(tMillis * baseFreq) * 400 + 500; // sine from ~100 to 900
    float noise = sin(tMillis * noiseFreq + random(0, 1000)) * noiseAmp;

    return constrain(base + noise, 0, 1023);
}
