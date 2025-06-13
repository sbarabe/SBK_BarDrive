/**
 * @file SBK_BarDrive.h
 * @brief High-level controller for animated bar meters and LED displays compatible with external SBK_MAX72xx or SBK_HT16K33 drivers.
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @copyright
 * Copyright (c) 2025 Samuel Barabé
 * 
 * Part of the SBK BarDrive Arduino Library
 * https://github.com/smartbuilds/SBK_BarDrive
 *
 * @version 1.0.0
 * @license MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 */

/**
 * @example animationShowcase.ino
 * This example shows how to initialize SBK_BarDrive with MAX72xx and run built-in animations.
 * 
 * @example customMappingDemo.ino
 * Demonstrates using SBK_BarDrive with a custom row/column mapping.
 * 
 * @example presetTypeDemo.ino
 * @brief Demonstrates using SBK_BarDrive with a preset BarMeterType mapping.
 * 
 * @example segmentCountDemo.ino
 * @brief Demonstrates using SBK_BarDrive with a segment count (auto-mapped).
 * 
 * @example makingYourOwnAnim.ino
 * @brief Example showing how to manually fill a bar meter based on analog input without built-in animations.
 * 
*/

#pragma once

/**
 * @def SBK_BARDRIVE_WITH_ANIM
 * @brief Enables built-in animation support for SBK_BarDrive.
 *
 * Define this macro **before including** `SBK_BarDrive.h` to activate the `SBK_BarMeterAnimations` system.
 * Without this definition, animation methods like `.animations()` will not be available,
 * reducing flash and RAM usage for minimal builds.
 */
#ifndef SBK_BARDRIVE_WITH_ANIM
#pragma message(" ⚠️ SBK_BARDRIVE_WITH_ANIM is not defined. SBK BarDrive Built-in animations are disabled and cannot be accessed, saving memory if they are not needed. If you do need the animations library, define SBK_BARDRIVE_WITH_ANIM before including SBK_BarDrive.h.")
#endif

/**
 * @def SBK_TRACK_STATE_IN_RAM
 * @brief Enables pixel state tracking in SBK_BarMeter.
 *
 * This option enables internal pixel state tracking in RAM, independent of the hardware display driver.
 * Use this if you need to query pixel states (e.g., via getPixelState()) before calling show(),
 * or if your logic depends on precise per-segment tracking not guaranteed by the driver.
 *
 * Leave undefined to reduce memory usage. Pixel state queries will fallback to reading the driver's 
 * internal buffer only returning the correct states after calling show().
 */
#ifndef SBK_TRACK_STATE_IN_RAM
#pragma message(" ⚠️ SBK_TRACK_STATE_IN_RAM is not defined. Pixel state tracking in SBK_BarMeter is disabled to save memory. getPixelState(pixel) will return the state from the driver's buffer, which may not reflect the latest animation logic until show() is called.")
#endif

// IMPORTANT: Include the appropriate driver before SBK_BarDrive.h
// e.g., #include <SBK_MAX72xx.h> or <SBK_HT16K33.h>
#if !defined(SBK_MAX72xx_IS_DEFINED) && !defined(SBK_HT16K33_IS_DEFINED)
#pragma message(" ⚠️ SBK BarDrive library note : Neither SBK_MAX72xx.h or SBK_HT16K33.h are included in the code. You need to include the file for your driver type and declare a driver before any bar meter instance can be created.")
#endif

#include <Arduino.h>
#ifdef SBK_BARDRIVE_WITH_ANIM
#include "SBK_BarMeterAnimations.h"
#endif

/**
 * @enum BarMeterType
 * @brief Preset types for common SBK BarMeter hardware configurations.
 *
 * Defines built-in mapping presets for different SBK bar meter PCBs.
 * These are used to simplify initialization of the `SBK_BarMeter` class
 * without manually specifying segment mappings.
 */
enum class BarMeterType
{
    DEFAULT_BarMeter,  ///< Generic fallback layout (8×8 grid).
    SBK_BarMeter_SK28, ///< Alias for **SBK BarMeter SK28** PCB using BL28_3005SK BarMeterType.
    SBK_BarMeter_SA28, ///< Alias for **SBK BarMeter SA28** PCB using BL28_3005SA BarMeterType.
    SBK_BarMeter_24,   ///< Alias for **SBK BarMeter 24** PCB 3×8 layout for 24-segment bar.
    SBK_BarMeter_40,   ///< Alias for **SBK BarMeter 40** PCB 5×8 layout for 40-segment bar.
    BL28_3005SK,       ///< Native mapping for BL28(Z)-3005SK bar meter (7×4 layout, common cathode).
    BL28_3005SA,       ///< Native mapping for BL28(Z)-3005SA bar meter (4×7 layout, common anode).
    CUSTOM = 255       ///< User-supplied mapping (via row/col array).
};

/**
 * @enum BarDirection
 * @brief Direction of bar fill or animation progression.
 *
 * Used to control the rendering orientation of bar animations and pixel updates.
 * - `FORWARD`: From first segment to last.
 * - `REVERSE`: From last segment to first.
 */
enum class BarDirection : uint8_t
{
    FORWARD = 0, ///< From first segment to last.
    REVERSE = 1  ///< From last segment to first.
};

/**
 * @class SBK_BarMeter
 * @brief Template class for segment-based bar meter control using row/column LED mappings.
 *
 * Supports configurable segment mapping, direction inversion, and optional pixel state tracking.
 * Compatible with both MAX72xx and HT16K33 backends.
 */
template <typename DriverT>
class SBK_BarMeter
{
public:
    /**
     * @brief Construct a SBK_BarMeter using a preset bar type.
     *
     * @param driver       Pointer to the LED driver (MAX72xx or HT16K33).
     * @param deviceIndex  Index of the device in the chain.
     * @param type         BarMeterType enum defining a preset mapping.
     * @param direction    Optional direction (FORWARD or REVERSE).
     */
    SBK_BarMeter(DriverT *driver,
                 uint8_t deviceIndex,
                 BarMeterType type = BarMeterType::BL28_3005SK,
                 BarDirection direction = BarDirection::FORWARD)
        : _driver(driver),
          _deviceIndex(deviceIndex),
          _direction(direction),
          _type(type)
    {
        _initializeMapping(type);
    }

    /**
     * @brief Construct a SBK_BarMeter with a given number of vertical segments.
     *
     * @param driver       Pointer to the LED driver.
     * @param deviceIndex  Index of the device in the chain.
     * @param numSegments  Number of segments in the bar meter.
     * @param direction    Optional direction (FORWARD or REVERSE).
     */
    SBK_BarMeter(DriverT *driver,
                 uint8_t deviceIndex,
                 uint8_t numSegments,
                 BarDirection direction = BarDirection::FORWARD)
        : _driver(driver),
          _deviceIndex(deviceIndex),
          _numSegments(numSegments),
          _direction(direction)
    {
        _type = BarMeterType::DEFAULT_BarMeter;
        _initializeMapping(_type);
    }

    /**
     * @brief Construct a SBK_BarMeter with a custom pixel [row, col] mapping.
     *
     * @tparam N           Number of segments (inferred from array size).
     * @param driver       Pointer to the LED driver.
     * @param deviceIndex  Index of the device.
     * @param mapping      Array of [row, col] pairs for segment mapping.
     * @param direction    Bar animation direction.
     * @param progmem      Whether mapping is stored in PROGMEM.
     */
    template <size_t N>
    SBK_BarMeter(DriverT *driver,
                 uint8_t deviceIndex,
                 const uint8_t (&mapping)[N][2],
                 BarDirection direction = BarDirection::FORWARD,
                 bool progmem = false)
        : _driver(driver),
          _deviceIndex(deviceIndex),
          _customMapping(mapping),
          _direction(direction),
          _userMappingIsProgmem(progmem)
    {
        _type = BarMeterType::CUSTOM;
        _numSegments = N;
    }

    ~SBK_BarMeter()
    {
#ifdef SBK_TRACK_STATE_IN_RAM
        if (_pixelStateCache)
        {
            delete[] _pixelStateCache;
            _pixelStateCache = nullptr;
        }
#endif
    }

    /**
     * @brief Push the current state to the display.
     */
    void show() { _driver->show(_deviceIndex); }

    /**
     * @brief Clear all bar segments.
     */
    void clear()
    {
        for (uint8_t i = 0; i < _numSegments; ++i)
            setPixel(i, false);
    }

    /**
     * @brief Set the bar fill direction.
     * @param dir New direction (FORWARD or REVERSE).
     */
    void setDirection(BarDirection dir) { _direction = dir; }

    /**
     * @brief Get the current bar fill direction.
     * @return BarDirection enum.
     */
    BarDirection getDirection() const { return _direction; }

    /**
     * @brief Get the total number of segments in the bar.
     * @return Number of segments.
     */
    uint8_t getSegNum() const { return _numSegments; }

    /**
     * @brief Set a segment's LED on or off.
     *
     * @param segment  Index of the segment to modify.
     * @param state    true to turn on, false to turn off.
     */
    void setPixel(uint8_t segment, uint8_t state)
    {
        if (segment >= _numSegments)
            return;
        if (_direction == BarDirection::REVERSE)
            segment = (_numSegments - 1) - segment;

        uint8_t row, col;
        _getMappedRowCol(segment, &row, &col);
        _driver->setLed(_deviceIndex, row, col, state != 0);

#ifdef SBK_TRACK_STATE_IN_RAM
        if (_pixelStateCache)
            _pixelStateCache[segment] = state;
#endif
    }

    /**
     * @brief Get the current state of a bar segment (pixel).
     *
     * This function returns the last known ON/OFF state of a segment.
     * If SBK_TRACK_STATE_IN_RAM is defined, it uses the internal state buffer.
     * Otherwise, it queries the driver’s internal buffer (not the physical IC).
     *
     * Direction and segment mapping are handled internally, so the returned
     * value always corresponds to the logical segment number.
     *
     * @param segment Index of the segment to query (0 to _numSegments - 1).
     * @return 1 if the segment is ON, 0 if OFF, or 0 if invalid or driver unavailable.
     */
    uint8_t getPixelState(uint8_t segment) const
    {
        if (segment >= _numSegments)
            return false;

#ifdef SBK_TRACK_STATE_IN_RAM
        if (_direction == BarDirection::REVERSE)
            segment = (_numSegments - 1) - segment;

        return _pixelStateCache ? _pixelStateCache[segment] : false;
#else
        if (!_driver)
            return false;

        uint8_t row, col;
        _getMappedRowCol(segment, &row, &col);
        return _driver->getLed(_deviceIndex, row, col);
#endif
    }

private:
    void _initializeMapping(BarMeterType type)
    {
        if (type == BarMeterType::SBK_BarMeter_SK28)
            type = BarMeterType::BL28_3005SK;
        if (type == BarMeterType::SBK_BarMeter_SA28)
            type = BarMeterType::BL28_3005SA;

        switch (type)
        {
        case BarMeterType::DEFAULT_BarMeter:
            _rowsNum = 8;
            _colsNum = 8;
            break;
        case BarMeterType::BL28_3005SK:
            _numSegments = 28;
            _rowsNum = 7;
            _colsNum = 4;
            break;
        case BarMeterType::BL28_3005SA:
            _numSegments = 28;
            _rowsNum = 4;
            _colsNum = 7;
            break;
        case BarMeterType::SBK_BarMeter_24:
            _numSegments = 24;
            _rowsNum = 3;
            _colsNum = 8;
            break;
        case BarMeterType::SBK_BarMeter_40:
            _numSegments = 40;
            _rowsNum = 5;
            _colsNum = 8;
            break;
        case BarMeterType::CUSTOM:
            break;
        default:
            _numSegments = 64;
            _rowsNum = 8;
            _colsNum = 8;
            break;
        }

// Allocate pixel state cache
#ifdef SBK_TRACK_STATE_IN_RAM
        if (_pixelStateCache)
            delete[] _pixelStateCache;
        _pixelStateCache = new bool[_numSegments]{};
#endif
    }

    void _getMappedRowCol(uint8_t index, uint8_t *row, uint8_t *col) const
    {
        uint8_t r, c;
        if (_type == BarMeterType::CUSTOM && _customMapping)
        {
            if (_userMappingIsProgmem)
            {
                r = pgm_read_byte(&_customMapping[index][0]);
                c = pgm_read_byte(&_customMapping[index][1]);
            }
            else
            {
                r = _customMapping[index][0];
                c = _customMapping[index][1];
            }
        }
        else
        {
            if (_direction == BarDirection::REVERSE)
                index = (_numSegments - 1) - index;
            r = index / _colsNum;
            c = index % _colsNum;
        }
        *row = r;
        *col = c;
    }

    DriverT *_driver;
    const uint8_t _deviceIndex;
    BarMeterType _type;
    BarDirection _direction;
    uint8_t _numSegments;
    const uint8_t (*_customMapping)[2] = nullptr;
    uint8_t _rowsNum = 0;
    uint8_t _colsNum = 0;
    bool _userMappingIsProgmem = false;
#ifdef SBK_TRACK_STATE_IN_RAM
    bool *_pixelStateCache = nullptr;
#endif
};

// -----------------------------
// SBK_BarDrive wrapper
// -----------------------------
/**
 * @class SBK_BarDrive
 * @brief Unified interface combining SBK_BarMeter and optional animation support.
 *
 * Provides a simplified high-level interface to manipulate LED bar meters and trigger predefined animations,
 * including fills, pulses, signal tracking, and block effects. Requires `SBK_BARDRIVE_WITH_ANIM` to enable animation support.
 */
template <typename DriverT>
class SBK_BarDrive
{
public:
    /**
     * @brief Construct a SBK_BarDrive using a preset bar meter type.
     *
     * @param driver       Pointer to the LED driver.
     * @param deviceIndex  Index of the LED driver chip.
     * @param type         Preset type of bar layout.
     * @param direction    Display fill direction.
     */
    SBK_BarDrive(DriverT *driver,
                 uint8_t deviceIndex,
                 BarMeterType type,
                 BarDirection direction = BarDirection::FORWARD)
        : _barMeter(driver, deviceIndex, type, direction)
#ifdef SBK_BARDRIVE_WITH_ANIM
          ,
          _barAnimations(_barMeter)
#endif
    {
#ifdef SBK_BARDRIVE_WITH_ANIM
        _barAnimations.setNumSegments(_barMeter.getSegNum());
#endif
    }

    /**
     * @brief Construct a SBK_BarDrive using a given number of vertical segments.
     *
     * @param driver       Pointer to the LED driver.
     * @param deviceIndex  Index of the device in the chain.
     * @param numSegments  Number of segments in the bar meter.
     * @param direction    Optional direction (FORWARD or REVERSE).
     */
    SBK_BarDrive(DriverT *driver,
                 uint8_t deviceIndex,
                 uint8_t numSegments,
                 BarDirection direction = BarDirection::FORWARD)
        : _barMeter(driver, deviceIndex, numSegments, direction)
#ifdef SBK_BARDRIVE_WITH_ANIM
          ,
          _barAnimations(_barMeter)
#endif
    {
#ifdef SBK_BARDRIVE_WITH_ANIM
        _barAnimations.setNumSegments(_barMeter.getSegNum());
#endif
    }

    /**
     * @brief Construct a SBK_BarDrive using a custom pixel [row, col] mapping.
     *
     * @tparam N           Number of segments (inferred from array size).
     * @param driver       Pointer to the LED driver.
     * @param deviceIndex  Index of the device.
     * @param mapping      Array of [row, col] pairs for segment mapping.
     * @param direction    Bar animation direction.
     * @param progmem      Whether mapping is stored in PROGMEM.
     */
    template <size_t N>
    SBK_BarDrive(DriverT *driver,
                 uint8_t deviceIndex,
                 const uint8_t (&mapping)[N][2],
                 BarDirection direction = BarDirection::FORWARD,
                 bool progmem = false)
        : _barMeter(driver, deviceIndex, mapping, direction, progmem)
#ifdef SBK_BARDRIVE_WITH_ANIM
          ,
          _barAnimations(_barMeter)
#endif
    {
#ifdef SBK_BARDRIVE_WITH_ANIM
        _barAnimations.setNumSegments(_barMeter.getSegNum());
#endif
    }

    /**
     * @brief Get the underlying SBK_BarMeter instance.
     * @return Reference to the SBK_BarMeter object.
     */
    SBK_BarMeter<DriverT> &barmeter() { return _barMeter; }
#ifdef SBK_BARDRIVE_WITH_ANIM
    /**
     * @brief Get the animation controller instance (if enabled).
     * @return Reference to SBK_BarMeterAnimations.
     */
    SBK_BarMeterAnimations<SBK_BarMeter<DriverT>> &animations() { return _barAnimations; }
#endif

    /**
     * @brief Push LED buffer to display.
     */
    void show() { _barMeter.show(); }

    /**
     * @brief Get number of bar segments.
     */
    uint8_t getSegNum() { return _barMeter.getSegNum(); }

    /**
     * @brief Set direction of bar fill.
     */
    void setDirection(BarDirection newDirection) { _barMeter.setDirection(newDirection); }

    /**
     * @brief Get current direction.
     */
    BarDirection getDirection() { return _barMeter.getDirection(); }

    /**
     * @brief Set a segment's LED on/off.
     */
    void setPixel(uint8_t segment, uint8_t state) { _barMeter.setPixel(segment, state); }

    /**
     * @brief Get pixel state (cached queried).
     *
     * @warning This method may return unpredictable or stale values unless
     * `SBK_TRACK_STATE_IN_RAM` is defined in your sketch to enable internal
     * pixel state buffering.
     *
     * @param segment Index of the segment to query.
     * @return 1 if lit, 0 if off, false if disabled or invalid.
     */
    uint8_t getPixelState(uint8_t segment) { return _barMeter.getPixelState(segment); }

private:
    SBK_BarMeter<DriverT> _barMeter;
#ifdef SBK_BARDRIVE_WITH_ANIM
    /**
     * @brief Get the animation controller instance (if enabled).
     * @return Reference to SBK_BarMeterAnimations.
     */
    SBK_BarMeterAnimations<SBK_BarMeter<DriverT>> _barAnimations;
#endif
};
