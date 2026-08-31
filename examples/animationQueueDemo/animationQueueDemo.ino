/**
 * @file animationQueueDemo.ino
 * @brief Demonstrates queued animations, live logic inversion, and wait().
 *
 * This example demonstrates:
 *      - Capturing four animations in a fixed-storage queue
 *      - Inspecting the active queue index
 *      - Modifying a running animation without losing its block positions
 *      - Stopping block emission and allowing active blocks to flow out
 *      - Adding a non-blocking pause with wait()
 *
 * Requirements:
 *      - Supported driver with compatible library (SBK_MAX72xx or SBK_HT16K33)
 *      - Bar meter display or LED array wired to the selected driver
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @version 2.1.0
 * @license MIT
 */

#include <Arduino.h>

// -----------------------------------------------------------------------------
// SBK BarDrive Configuration Flags
// -----------------------------------------------------------------------------
#define SBK_BARDRIVE_WITH_ANIM // Enable preset animations and queue controls.

// This example uses four entries. The library default is already four.
// If you change the global queue capacity, keep it at four or greater here.
// #define SBK_BARDRIVE_QUEUE_CAPACITY 4

// -----------------------------------------------------------------------------
// SELECT YOUR DRIVER SETUP
// Enable one driver configuration and comment out the other two.
// -----------------------------------------------------------------------------

/* === [A] MAX7219/MAX7221 via SOFTWARE SPI (any 3 digital pins) === */
#define DIN_PIN A4 // Software SPI Data In
#define CLK_PIN A5 // Software SPI Clock
#define CS_PIN A3  // Chip Select / LOAD
#include <SBK_MAX72xxSoft.h>
SBK_MAX72xxSoft driver(DIN_PIN, CLK_PIN, CS_PIN, 1);
#include <SBK_BarDrive.h>
SBK_BarDrive<SBK_MAX72xxSoft> bar(
    &driver, 0, MatrixPreset::SBK_BarMeter_SK28);

/* === [B] MAX7219/MAX7221 via HARDWARE SPI (dedicated MCU SPI pins) === */
// #define CS_PIN A3 // Chip Select / LOAD
// #include <SBK_MAX72xxHard.h>
// SBK_MAX72xxHard driver(CS_PIN, 1);
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_MAX72xxHard> bar(
//     &driver, 0, MatrixPreset::SBK_BarMeter_SK28);

/* === [C] HT16K33 via I2C === */
// #include <SBK_HT16K33.h>
// const uint8_t NUM_DEV = 1;
// const uint8_t DEV0_IDX = 0;
// const uint8_t DEV0_ADD = 0x70;
// const uint8_t DEV0_NUM_ROWS = 8; // 20-SOP = 8, 24-SOP = 12, 28-SOP = 16
// SBK_HT16K33 driver(NUM_DEV);
// #include <SBK_BarDrive.h>
// SBK_BarDrive<SBK_HT16K33> bar(
//     &driver, 0, MatrixPreset::SBK_BarMeter_SK28);

/*
 * The default setup uses the SBK BarMeter SK28 28-segment preset.
 * Change the MatrixPreset or constructor to match your display and wiring.
 */

uint8_t previousQueueIndex = 0xFF; // Same value as NO_QUEUE_INDEX.
uint32_t queueStepStartedAt = 0;
bool blockLogicInverted = false;

void startAnimationQueue()
{
    /*
     * enqueue() captures the animation and its modifiers in the next queue slot.
     * The captured animations do not start until startQueue() is called.
     *
     * Queue index 0: fill the display.
     * Queue index 1: empty the display.
     * Queue index 2: emit colliding blocks indefinitely (numBlocks defaults to 0).
     * Queue index 3: preserve the empty display for 1000 ms without blocking.
     *
     * wait() is different from Arduino delay(): loop() and animations.update()
     * continue to run, so the rest of the application remains responsive.
     */
    bar.animations().stop()
        .fillUpIntv(35).noLoop().enqueue()
        .emptyDownIntv(35).noLoop().enqueue()
        .collidingBlocks(45, 4, 2).enqueue()
        .wait(1000).enqueue()
        .startQueue();
}

void setup()
{
#ifdef SBK_HT16K33_IS_DEFINED
    // HT16K33 setup is required only when configuration [C] is enabled.
    driver.setAddress(DEV0_IDX, DEV0_ADD);
    driver.setDriverRows(DEV0_IDX, DEV0_NUM_ROWS);
#endif

    driver.begin();
    driver.setBrightness(0, 10);
    bar.setDirection(BarDirection::FORWARD);
    startAnimationQueue();
}

void loop()
{
    // update() advances timing and queue transitions; show() sends pixels to the driver.
    bar.animations().update();
    bar.show();

    /*
     * Detect entry into a new queue slot so each timed action is measured from
     * the start of that animation, rather than from the start of the sketch.
     */
    const uint8_t currentQueueIndex = bar.animations().currentQueueIndex();
    if (currentQueueIndex != previousQueueIndex)
    {
        previousQueueIndex = currentQueueIndex;
        queueStepStartedAt = millis();
        blockLogicInverted = false;
    }

    const uint32_t queueStepElapsed = millis() - queueStepStartedAt;

    /*
     * invertLogic() is intentionally called outside the queue. Loading another
     * queued animation would reinitialize the animation and lose all current
     * block positions. Modifying the active index-2 animation in place keeps
     * the same blocks moving while changing how they are rendered.
     *
     * The boolean prevents invertLogic() from being called on every loop pass.
     */
    if (bar.animations().isQueueIndexPlaying(2) &&
        queueStepElapsed >= 3000 &&
        !blockLogicInverted)
    {
        bar.animations().invertLogic();
        blockLogicInverted = true;
    }

    /*
     * After three more seconds, stop creating blocks. Active blocks continue
     * moving until they leave the display. The animation then completes
     * naturally and the queue advances to wait(1000).
     */
    if (bar.animations().isQueueIndexPlaying(2) &&
        queueStepElapsed >= 6000 &&
        bar.animations().isBlockEmissionEnabled())
    {
        bar.animations().stopBlockEmission();
    }

    // After the final wait entry completes, rebuild and replay the demonstration.
    if (!bar.animations().isRunning())
        startAnimationQueue();
}
