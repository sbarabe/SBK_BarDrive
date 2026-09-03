/**
 * @file animationQueueDemo.ino
 * @brief Demonstrates queued animations, seamless state handoff, and wait().
 *
 * This example demonstrates:
 *      - Capturing four animations in a fixed-storage queue
 *      - Applying a hard time limit to a queued animation
 *      - Switching compatible animations without resetting their block positions
 *      - Stopping block emission on a timer and draining active blocks
 *      - Adding a non-blocking pause with wait()
 *
 * Requirements:
 *      - Supported driver with compatible library (SBK_MAX72xx or SBK_HT16K33)
 *      - Bar meter display or LED array wired to the selected driver
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @version 2.1.2
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

void startAnimationQueue()
{
    /*
     * enqueue() captures the animation and its modifiers in the next queue slot.
     * The captured animations do not start until startQueue() is called.
     *
     * Queue index 0: run one complete fill-up/fill-down bounce.
     * Queue index 1: emit colliding blocks for a maximum of three seconds.
     * Queue index 2: reverse the in-flight blocks, emit outward for three
     *                seconds, then drain all remaining blocks naturally.
     * Queue index 3: preserve the empty display for 1000 ms without blocking.
     *
     * Modifier ordering:
     * - loop()/noLoop() configure an animation before it is captured.
     * - forTime() may be used before enqueue(), or immediately afterward to
     *   modify the most recently queued entry, as shown for queue index 1.
     * - enqueueReverseAnim() captures a reversed copy of the preceding entry
     *   while preserving its trackers and active block positions.
     * - stopBlockEmissionAfter() may also follow an enqueue operation. It stops
     *   new blocks at the deadline, disables looping, and allows active blocks
     *   to leave before advancing the queue.
     * - wait() is always non-looping, so noLoop() is unnecessary for waits.
     *
     * wait() is different from Arduino delay(): loop() and animations.update()
     * continue to run, so the rest of the application remains responsive.
     */
    bar.animations().stop()
        .bounceFillUpIntv(35, 35).noLoop().enqueue()
        .collidingBlocks(45, 4, 2, 0).enqueue().forTime(3000)
        .enqueueReverseAnim().stopBlockEmissionAfter(3000)
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

    // After the final wait entry completes, rebuild and replay the demonstration.
    if (!bar.animations().isRunning())
        startAnimationQueue();
}
