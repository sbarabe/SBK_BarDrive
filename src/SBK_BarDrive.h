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
 * @version 2.0.4
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
 * @brief Demonstrates using SBK_BarDrive with a preset MatrixPreset mapping.
 *
 * @example segmentCountDemo.ino
 * @brief Demonstrates SBK_BarDrive using only segment count for 1D bar meter type (auto-mapped).
 *
 * @example makingYourOwnAnim.ino
 * @brief Example showing how to manually fill a bar meter based on analog input without built-in animations.
 *
 * @example splitDriverDevicesBarMeter.ino
 * @brief Example showing how to split a bar meter on multiple driver devices.
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

// IMPORTANT: Include the appropriate driver before SBK_BarDrive.h
// e.g., #include <SBK_MAX72xxSoft.h>, <SBK_MAX72xxHard.h> or <SBK_HT16K33.h>
#if !defined(SBK_MAX72xx_IS_DEFINED) && !defined(SBK_HT16K33_IS_DEFINED)
#pragma message(" ⚠️ SBK BarDrive library note : Neither SBK_MAX72xx.h or SBK_HT16K33.h are included in the code. You need to include the file for your driver matrixPreset and declare a driver before any bar meter instance can be created.")
#endif

#include <Arduino.h>
#ifdef SBK_BARDRIVE_WITH_ANIM
#include "SBK_BarMeterAnimations.h"
#endif

/**
 * @enum MatrixPreset
 * @brief Preset types for common matrix-style bar meter hardware configurations.
 *
 * Defines built-in mapping presets for SBK matrix-type bar meters.
 * These presets simplify the initialization of the `SBK_BarMeter` class
 * by providing predefined segment-to-row/column mappings, avoiding the need for manual setup.
 */
enum class MatrixPreset
{
    NONE,              ///< No preset selected; manual segment mapping is required.
    SBK_BarMeter_SK28, ///< Alias for SBK BarMeter SK28, uses the BL28_3005SK preset internally.
    SBK_BarMeter_SA28, ///< Alias for SBK BarMeter SA28, uses the BL28_3005SA preset internally.
    BL28_3005SK,       ///< Native mapping for BL28-3005SK (4 anodes × 7 cathodes), common cathode.
    BL28_3005SA        ///< Native mapping for BL28-3005SA (7 anodes × 4 cathodes), common anode.
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
 * @brief Template class for controlling segment-based LED bar meters using row/column mappings.
 *
 * This class provides flexible control over bar meter displays by mapping logical segments
 * to physical LED positions defined by [device, row, col] coordinates. It supports:
 * - Direction inversion (FORWARD or REVERSE)
 * - Custom or preset segment layouts
 * - Matrix-style or linear segment addressing
 * - Split bar meters spanning multiple devices of the same driver type
 *
 * Compatible with any driver that implements the SBK display driver API,
 * including SBK_MAX72xx (hardware/software SPI) and SBK_HT16K33 (I2C).
 *
 * Typically used via the higher-level `SBK_BarDrive` wrapper for simplified control and animation support.
 */
template <typename DriverT>
class SBK_BarMeter
{
public:
    /**
     * @brief Construct a SBK_BarMeter using a predefined matrix-style layout.
     *
     * @param driver        Pointer to the LED driver (e.g., SBK_MAX72xx or SBK_HT16K33).
     * @param devIdx        Index of the first device in the chain (0–7).
     * @param matrixPreset  Preset mapping type from the MatrixPreset enum.
     * @param direction     Bar fill direction (FORWARD or REVERSE). Default is FORWARD.
     * @param rowOffset     Optional row offset (if the matrix is not wired to the first row). Default is 0.
     * @param colOffset     Optional column offset (if the matrix is not wired to the first column). Default is 0.
     *
     * This constructor simplifies setup by using a predefined segment-to-row/column mapping,
     * useful for SBK matrix-type bar meters such as SK28 or SA28 models.
     */
    SBK_BarMeter(DriverT *driver,
                 uint8_t devIdx,
                 MatrixPreset matrixPreset = MatrixPreset::BL28_3005SK,
                 BarDirection direction = BarDirection::FORWARD,
                 uint8_t rowOffset = 0,
                 uint8_t colOffset = 0)
        : _driver(driver),
          _devIdx(constrain(devIdx, 0, 7)),
          _matrixPreset(matrixPreset),
          _direction(direction)
    {
        _isMatrixMapped = true;
        if (_devIdx > (driver->devsNum() - 1))
        {
            _initializePresetMapping(MatrixPreset::NONE);
            _segsNum = 0;
            _rowsNum = 0;
            _colsNum = 0;
            _rowOffset = 0;
            _colOffset = 0;
        }
        else
        {
            _initializePresetMapping(matrixPreset);
            _rowOffset = constrain(rowOffset, 0, _driver->maxRows(_devIdx) - 1);
            _colOffset = constrain(colOffset, 0, _driver->maxColumns() - 1);
        }
    }

    /**
     * @brief Construct a SBK_BarMeter with a custom matrix size (rows × columns layout).
     *
     * @param driver     Pointer to the LED driver (e.g., SBK_MAX72xx or SBK_HT16K33).
     * @param devIdx     Index of the first device in the chain (0–7).
     * @param rowsNum    Number of matrix rows (anodes). May span multiple devices vertically.
     * @param colsNum    Number of matrix columns (cathodes). Limited to the column capacity of a single device.
     * @param direction  Optional bar fill direction (FORWARD or REVERSE). Default is FORWARD.
     * @param rowOffset  Optional row offset, if the matrix does not begin on the first driver row. Default is 0.
     * @param colOffset  Optional column offset, if the matrix does not begin on the first driver column. Default is 0.
     *
     * This constructor allows you to define a matrix-style bar layout with a custom size and origin,
     * without using a predefined MatrixPreset.
     *
     * ⚠️ Note: Columns cannot be split across devices. Only rows may be extended vertically over multiple devices.
     * Attempting to configure more columns than the driver hardware supports will result in clamping.
     *
     * Useful for adapting custom matrix layouts or hardware not covered by presets.
     */
    SBK_BarMeter(DriverT *driver,
                 uint8_t devIdx,
                 uint8_t rowsNum,
                 uint8_t colsNum,
                 BarDirection direction = BarDirection::FORWARD,
                 uint8_t rowOffset = 0,
                 uint8_t colOffset = 0)
        : _driver(driver),
          _devIdx(constrain(devIdx, 0, 7)),
          _direction(direction)
    {
        _isMatrixMapped = true;
        if (_devIdx > (driver->devsNum() - 1))
        {
            _segsNum = 0;
            _rowsNum = 0;
            _colsNum = 0;
            _rowOffset = 0;
            _colOffset = 0;
        }
        else
        {
            _rowsNum = constrain(rowsNum, 1, 255);
            _colsNum = constrain(colsNum, 1, driver->maxColumns());
            _rowOffset = constrain(rowOffset, 0, _driver->maxRows(_devIdx) - 1);
            _colOffset = constrain(colOffset, 0, _driver->maxColumns() - 1);
        }
    }

    /**
     * @brief Construct a SBK_BarMeter with a fixed number of vertical segments (non-matrix layout).
     *
     * @param driver     Pointer to the LED driver (e.g., SBK_MAX72xx or SBK_HT16K33).
     * @param devIdx     Index of the device in the chain (0–7).
     * @param segsNum    Number of logical segments (LEDs) in the bar meter.
     * @param direction  Optional bar fill direction (FORWARD or REVERSE). Default is FORWARD.
     * @param segOffset  Optional segment offset if the bar does not start at the first LED output. Default is 0.
     *
     * This constructor is intended for 1D segment-based bar displays, such as linear bars where
     * each segment is addressed sequentially (not via a 2D matrix).
     *
     * The `segOffset` allows you to skip physical outputs if the display is not connected to the first pin
     * of the device. This is useful when sharing a device across multiple bar segments or skipping unused lines.
     *
     * ⚠️ If the specified device index exceeds the available device count, the bar meter is initialized as empty.
     */
    SBK_BarMeter(DriverT *driver,
                 uint8_t devIdx,
                 uint8_t segsNum,
                 BarDirection direction = BarDirection::FORWARD,
                 uint8_t segOffset = 0)
        : _driver(driver),
          _devIdx(constrain(devIdx, 0, 7)),
          _segsNum(segsNum),
          _direction(direction)
    {
        _isMatrixMapped = false;
        if (_devIdx > (driver->devsNum() - 1))
        {
            _segsNum = 0;
            _rowsNum = 0;
            _colsNum = 0;
            _segOffset = 0;
        }
        else
        {
            _rowsNum = _driver->maxRows(_devIdx);
            _colsNum = _driver->maxColumns();
            _segOffset = constrain(segOffset, 0, _driver->maxSegments(_devIdx) - 1);
        }
    }

    /**
     * @brief Construct a SBK_BarMeter using a custom pixel mapping array.
     *
     * @tparam N           Number of segments (inferred automatically from array size).
     * @param driver       Pointer to the LED driver (e.g., SBK_MAX72xx or SBK_HT16K33).
     * @param devIdx       Index of the first device in the chain (0–7).
     * @param mapping      Array of [device, row, col] tuples defining the physical LED position for each segment.
     * @param direction    Optional bar fill direction (FORWARD or REVERSE). Default is FORWARD.
     * @param progmem      Set to `true` if the mapping array is stored in PROGMEM (Flash memory). Default is `false`.
     * @param rowOffset    Optional offset to apply to all mapped row indices. Default is 0.
     * @param colOffset    Optional offset to apply to all mapped column indices. Default is 0.
     *
     * This constructor gives you full control over how each logical segment maps to a specific
     * LED location on a matrix or linear display, across one or more devices. It is ideal for:
     * - Non-standard matrix layouts
     * - Discontinuous or irregular bar patterns
     * - Hybrid/multi-device configurations
     *
     * ⚠️ If the specified `devIdx` is invalid (beyond the available devices), the bar is initialized as empty.
     */
    template <size_t N>
    SBK_BarMeter(DriverT *driver,
                 uint8_t devIdx,
                 const uint8_t (&mapping)[N][3],
                 BarDirection direction = BarDirection::FORWARD,
                 bool progmem = false,
                 uint8_t rowOffset = 0,
                 uint8_t colOffset = 0)
        : _driver(driver),
          _devIdx(constrain(devIdx, 0, 7)),
          _customMapping(mapping),
          _direction(direction),
          _userMappingIsProgmem(progmem)
    {
        _isMatrixMapped = true;
        if (_devIdx > (driver->devsNum() - 1))
        {
            _segsNum = 0;
            _rowsNum = 0;
            _colsNum = 0;
            _rowOffset = 0;
            _colOffset = 0;
        }
        else
        {
            _segsNum = N;
            _rowsNum = _driver->maxRows(_devIdx);
            _colsNum = _driver->maxColumns();
            _rowOffset = constrain(rowOffset, 0, _driver->maxRows(_devIdx) - 1);
            _colOffset = constrain(colOffset, 0, _driver->maxColumns() - 1);
        }
    }

    ~SBK_BarMeter() { /* Nothing to clean up for now*/ }

    /**
     * @brief Push the current LED state buffer to the physical display.
     *
     * This flushes the internal buffer of the underlying driver to all connected devices —
     * not just the one associated with this bar meter instance — making any recent changes
     * (e.g., via `setPixel()` or `clear()`) visible on the actual hardware.
     *
     * Internally calls `_driver->show()` to update the display.
     */
    void show() { _driver->show(); }

    /**
     * @brief Clear all bar segments.
     */
    void clear()
    {
        for (uint8_t i = 0; i < _segsNum; ++i)
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
    uint8_t getSegsNum() const { return _segsNum; }

    /**
     * @brief Print the segment-to-device mapping for debugging purposes.
     *
     * This function outputs the resolved mapping of each logical segment index
     * to its corresponding [device, row, column] LED coordinates, based on the internal
     * configuration (preset, custom map, or auto-mapped layout).
     *
     * Useful for verifying how segments are translated into physical driver outputs,
     * especially when using segment offsets, matrix presets, or split-device configurations.
     *
     * @param stream Reference to a `Stream` object (e.g., `Serial`) for printing output.
     *               Defaults to `Serial` if not specified.
     */
    void debugSegmentMapping(Stream &stream = Serial)
    {
        for (uint8_t i = 0; i < _segsNum; ++i)
        {
            uint8_t devIdx = _devIdx;
            uint8_t rowIdx, colIdx;
            _getMappedDevRowCol(i, &devIdx, &rowIdx, &colIdx);
            stream.print(F("Segment "));
            stream.print(i);
            stream.print(F(" → Device "));
            stream.print(devIdx);
            stream.print(F(", Row "));
            stream.print(rowIdx);
            stream.print(F(", Col "));
            stream.println(colIdx);
        }
    }

    /**
     * @brief Set the on/off state of a logical segment (LED).
     *
     * @param segment Index of the logical segment to control (0 to `getSegNum() - 1`).
     * @param state   true to turn the LED on, false to turn it off.
     *
     * This function resolves the segment index to its corresponding [device, row, col]
     * location and updates the internal buffer of the underlying driver.
     * The change will not be visible until `show()` is called.
     */

    void setPixel(uint8_t segment, uint8_t state)
    {
        if (segment >= _segsNum || !_driver)
            return;

        uint8_t devIdx = _devIdx;
        uint8_t rowIdx, colIdx;
        _getMappedDevRowCol(segment, &devIdx, &rowIdx, &colIdx);
        _driver->setLed(devIdx, rowIdx, colIdx, state != 0);
    }

    /**
     * @brief Get the current state of a bar segment (pixel).
     *
     * This function returns the last known ON/OFF state of a segment.
     * It queries the driver’s internal buffer (not the physical IC).
     *
     * Direction and segment mapping are handled internally, so the returned
     * value always corresponds to the logical segment number.
     *
     * @param segment Index of the segment to query (0 to _segsNum - 1).
     * @return 1 if the segment is ON, 0 if OFF, or 0 if invalid or driver unavailable.
     */
    uint8_t getPixelState(uint8_t segment) const
    {
        if (segment >= _segsNum || !_driver)
            return false;

        uint8_t devIdx = _devIdx;
        uint8_t rowIdx, colIdx;

        _getMappedDevRowCol(segment, &devIdx, &rowIdx, &colIdx);
        return _driver->getLed(devIdx, rowIdx, colIdx);
    }

    /**
     * @brief Apply a segment-wise offset (shifts segment index before mapping to row/col).
     * @param offset Number of segments to skip.
     * @return Reference to this instance.
     *
     * This offset is better used with a 1 to 1 ICs output to led display
     * where the display do not start on the first IC output.
     *
     */
    SBK_BarMeter &setSegmentOffset(uint8_t offset)
    {
        _segOffset = offset;
        return *this;
    }

    /**
     * @brief Apply a matrix-style physical offset for row/column positioning.
     * @param rowOffset Number of rows to shift downward.
     * @param colOffset Number of columns to shift rightward.
     * @return Reference to this instance.
     *
     *  Those offsets are better used with matrix led display matrixPreset, like BL28 matrixPreset
     *  where the matrix is not connected to the firsts IC row/column.
     *
     */
    SBK_BarMeter &setMatrixOffset(uint8_t rowOffset, uint8_t colOffset)
    {
        _rowOffset = rowOffset;
        _colOffset = colOffset;
        return *this;
    }

private:
    void _initializePresetMapping(MatrixPreset matrixPreset)
    {

        if (matrixPreset == MatrixPreset::SBK_BarMeter_SK28)
            matrixPreset = MatrixPreset::BL28_3005SK;
        if (matrixPreset == MatrixPreset::SBK_BarMeter_SA28)
            matrixPreset = MatrixPreset::BL28_3005SA;

        switch (matrixPreset)
        {
        case MatrixPreset::NONE:
            _rowsNum = _driver->maxRows(_devIdx);
            _colsNum = _driver->maxColumns();
            _segsNum = _rowsNum * _colsNum;
            _isMatrixMapped = false;
            break;
        case MatrixPreset::BL28_3005SK:
            _segsNum = 28;
            _rowsNum = 4;
            _colsNum = 7;
            _isMatrixMapped = true;
            break;
        case MatrixPreset::BL28_3005SA:
            _segsNum = 28;
            _rowsNum = 7;
            _colsNum = 4;
            _isMatrixMapped = true;
            break;
        default:
            _rowsNum = _driver->maxRows(_devIdx);
            _colsNum = _driver->maxColumns();
            _segsNum = _rowsNum * _colsNum;
            _isMatrixMapped = false;
            break;
        }
    }

    void _getMappedDevRowCol(uint8_t seg, uint8_t *devIdx, uint8_t *rowIdx, uint8_t *colIdx) const
    {
        // Apply direction correction
        uint8_t mappedSeg = (_direction == BarDirection::REVERSE)
                                ? (_segsNum - 1 - seg)
                                : seg;

        // Add segment offset only in segment-based mode
        mappedSeg += (_isMatrixMapped ? 0 : _segOffset);

        if (_customMapping)
        {
            // Handle custom mappings
            if (_userMappingIsProgmem)
            {
                *devIdx = pgm_read_byte(&_customMapping[mappedSeg][0]);
                *rowIdx = pgm_read_byte(&_customMapping[mappedSeg][1]);
                *colIdx = pgm_read_byte(&_customMapping[mappedSeg][2]);
            }
            else
            {
                *devIdx = _customMapping[mappedSeg][0];
                *rowIdx = _customMapping[mappedSeg][1] + _rowOffset;
                *colIdx = _customMapping[mappedSeg][2] + _colOffset;
            }
            return;
        }

        // Auto mapping

        const uint8_t segPerDev = _driver->maxRows(*devIdx) * _driver->maxColumns();
        const uint8_t baseDevIdx = *devIdx + (mappedSeg / segPerDev);
        const uint8_t localIdx = mappedSeg % segPerDev;

        if (_isMatrixMapped)
        {
            // Matrix-style layout: column-major mapping
            uint8_t baseRowIdx = localIdx % _rowsNum;
            uint8_t baseColIdx = localIdx / _rowsNum;

            *devIdx = baseDevIdx;
            *rowIdx = baseRowIdx + _rowOffset;
            *colIdx = baseColIdx + _colOffset;
        }
        else
        {
            // Segment-style layout: row-major linear index
            uint8_t baseRowIdx = localIdx / _driver->maxColumns();
            uint8_t baseColIdx = localIdx % _driver->maxColumns();

            *devIdx = baseDevIdx;
            *rowIdx = baseRowIdx;
            *colIdx = baseColIdx;
        }
    }

    DriverT *_driver;
    const uint8_t _devIdx;
    MatrixPreset _matrixPreset = MatrixPreset::NONE;
    const uint8_t (*_customMapping)[3] = nullptr;
    BarDirection _direction;
    uint8_t _segOffset = 0;       // existing logic
    uint8_t _rowOffset = 0;       // for matrix displays
    uint8_t _colOffset = 0;       // for matrix displays
    bool _isMatrixMapped = false; // whether to apply row/col offset
    uint8_t _segsNum;
    uint8_t _rowsNum = 0;
    uint8_t _colsNum = 0;
    bool _userMappingIsProgmem = false;
};

// -----------------------------
// SBK_BarDrive wrapper
// -----------------------------
/**
 * @class SBK_BarDrive
 * @brief Unified interface combining SBK_BarMeter logic with optional built-in animation support.
 *
 * SBK_BarDrive provides a simplified, high-level interface for controlling LED bar meters,
 * including segment fills, signal-driven displays, pulse effects, and block-based animations.
 *
 * Internally wraps an instance of `SBK_BarMeter` and, if enabled, provides access to animation
 * utilities via `SBK_BarMeterAnimations`.
 *
 * @note To enable animation features, define `SBK_BARDRIVE_WITH_ANIM` **before** including `SBK_BarDrive.h`.
 * If not defined, animation methods like `.animations()` will be excluded to save memory.
 */
template <typename DriverT>
class SBK_BarDrive
{
public:
    /**
     * @brief Construct a SBK_BarDrive using a predefined matrix-style layout.
     *
     * @param driver        Pointer to the LED driver (e.g., SBK_MAX72xx or SBK_HT16K33).
     * @param devIdx        Index of the first device in the chain (0–7).
     * @param matrixPreset  Preset mapping type from the MatrixPreset enum.
     * @param direction     Bar fill direction (FORWARD or REVERSE). Default is FORWARD.
     * @param rowOffset     Optional row offset (if the matrix is not wired to the first row). Default is 0.
     * @param colOffset     Optional column offset (if the matrix is not wired to the first column). Default is 0.
     *
     * This constructor simplifies setup by using a predefined segment-to-row/column mapping,
     * useful for SBK matrix-type bar meters such as SK28 or SA28 models.
     */
    SBK_BarDrive(DriverT *driver,
                 uint8_t devIdx,
                 MatrixPreset matrixPreset,
                 BarDirection direction = BarDirection::FORWARD,
                 uint8_t rowOffset = 0,
                 uint8_t colOffset = 0)
        : _barMeter(driver, devIdx, matrixPreset, direction, rowOffset, colOffset)
#ifdef SBK_BARDRIVE_WITH_ANIM
          ,
          _barAnimations(_barMeter)
#endif
    {
#ifdef SBK_BARDRIVE_WITH_ANIM
        _barAnimations.setSegsNum(_barMeter.getSegsNum());
#endif
    }

    /**
     * @brief Construct a SBK_BarDrive with a custom matrix size (rows × columns layout).
     *
     * @param driver     Pointer to the LED driver (e.g., SBK_MAX72xx or SBK_HT16K33).
     * @param devIdx     Index of the first device in the chain (0–7).
     * @param rowsNum    Number of matrix rows (anodes). May span multiple devices vertically.
     * @param colsNum    Number of matrix columns (cathodes). Limited to the column capacity of a single device.
     * @param direction  Optional bar fill direction (FORWARD or REVERSE). Default is FORWARD.
     * @param rowOffset  Optional row offset, if the matrix does not begin on the first driver row. Default is 0.
     * @param colOffset  Optional column offset, if the matrix does not begin on the first driver column. Default is 0.
     *
     * This constructor allows you to define a matrix-style bar layout with a custom size and origin,
     * without using a predefined MatrixPreset.
     *
     * ⚠️ Note: Columns cannot be split across devices. Only rows may be extended vertically over multiple devices.
     * Attempting to configure more columns than the driver hardware supports will result in clamping.
     *
     * Useful for adapting custom matrix layouts or hardware not covered by presets.
     */
    SBK_BarDrive(DriverT *driver,
                 uint8_t devIdx,
                 uint8_t rowsNum,
                 uint8_t colsNum,
                 BarDirection direction = BarDirection::FORWARD,
                 uint8_t rowOffset = 0,
                 uint8_t colOffset = 0)
        : _barMeter(driver, devIdx, rowsNum, colsNum, direction, rowOffset, colOffset)
#ifdef SBK_BARDRIVE_WITH_ANIM
          ,
          _barAnimations(_barMeter)
#endif
    {
#ifdef SBK_BARDRIVE_WITH_ANIM
        _barAnimations.setSegsNum(_barMeter.getSegsNum());
#endif
    }

    /**
     * @brief Construct a SBK_BarDrive with a fixed number of vertical segments (non-matrix layout).
     *
     * @param driver     Pointer to the LED driver (e.g., SBK_MAX72xx or SBK_HT16K33).
     * @param devIdx     Index of the device in the chain (0–7).
     * @param segsNum    Number of logical segments (LEDs) in the bar meter.
     * @param direction  Optional bar fill direction (FORWARD or REVERSE). Default is FORWARD.
     * @param segOffset  Optional segment offset if the bar does not start at the first LED output. Default is 0.
     *
     * This constructor is intended for 1D segment-based bar displays, such as linear bars where
     * each segment is addressed sequentially (not via a 2D matrix).
     *
     * The `segOffset` allows you to skip physical outputs if the display is not connected to the first pin
     * of the device. This is useful when sharing a device across multiple bar segments or skipping unused lines.
     *
     * ⚠️ If the specified device index exceeds the available device count, the bar meter is initialized as empty.
     */
    SBK_BarDrive(DriverT *driver,
                 uint8_t devIdx,
                 uint8_t segsNum,
                 BarDirection direction = BarDirection::FORWARD,
                 uint8_t segOffset = 0)
        : _barMeter(driver, devIdx, segsNum, direction, segOffset)
#ifdef SBK_BARDRIVE_WITH_ANIM
          ,
          _barAnimations(_barMeter)
#endif
    {
#ifdef SBK_BARDRIVE_WITH_ANIM
        _barAnimations.setSegsNum(_barMeter.getSegsNum());
#endif
    }

    /**
     * @brief Construct a SBK_BarDrive using a custom pixel mapping array.
     *
     * @tparam N           Number of segments (inferred automatically from array size).
     * @param driver       Pointer to the LED driver (e.g., SBK_MAX72xx or SBK_HT16K33).
     * @param devIdx       Index of the first device in the chain (0–7).
     * @param mapping      Array of [device, row, col] tuples defining the physical LED position for each segment.
     * @param direction    Optional bar fill direction (FORWARD or REVERSE). Default is FORWARD.
     * @param progmem      Set to `true` if the mapping array is stored in PROGMEM (Flash memory). Default is `false`.
     * @param rowOffset    Optional offset to apply to all mapped row indices. Default is 0.
     * @param colOffset    Optional offset to apply to all mapped column indices. Default is 0.
     *
     * This constructor gives you full control over how each logical segment maps to a specific
     * LED location on a matrix or linear display, across one or more devices. It is ideal for:
     * - Non-standard matrix layouts
     * - Discontinuous or irregular bar patterns
     * - Hybrid/multi-device configurations
     *
     * ⚠️ If the specified `devIdx` is invalid (beyond the available devices), the bar is initialized as empty.
     */
    template <size_t N>
    SBK_BarDrive(DriverT *driver,
                 uint8_t devIdx,
                 const uint8_t (&mapping)[N][3],
                 BarDirection direction = BarDirection::FORWARD,
                 bool progmem = false,
                 uint8_t rowOffset = 0,
                 uint8_t colOffset = 0)
        : _barMeter(driver, devIdx, mapping, direction, progmem, rowOffset, colOffset)
#ifdef SBK_BARDRIVE_WITH_ANIM
          ,
          _barAnimations(_barMeter)
#endif
    {
#ifdef SBK_BARDRIVE_WITH_ANIM
        _barAnimations.setSegsNum(_barMeter.getSegsNum());
#endif
    }

    ~SBK_BarDrive() { /* Nothing to clean up for now*/ }

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
     * @brief Push the current LED state buffer to the physical display.
     *
     * This flushes the internal buffer of the underlying driver to all connected devices —
     * not just the one associated with this bar meter instance — making any recent changes
     * (e.g., via `setPixel()` or `clear()`) visible on the actual hardware.
     *
     * Internally calls `_driver->show()` to update the display.
     */
    void show() { _barMeter.show(); }

    /**
     * @brief Clear all bar segments.
     */
    void clear() { _barMeter.clear(); }

    /**
     * @brief Set the bar fill direction.
     * @param dir New direction (FORWARD or REVERSE).
     */
    void setDirection(BarDirection dir) { _barMeter.setDirection(dir); }

    /**
     * @brief Get the current bar fill direction.
     * @return BarDirection enum.
     */
    BarDirection getDirection() { return _barMeter.getDirection(); }

    /**
     * @brief Get the total number of segments in the bar.
     * @return Number of segments.
     */
    uint8_t getSegsNum() { return _barMeter.getSegsNum(); }

    /**
     * @brief Print the segment-to-device mapping for debugging purposes.
     *
     * This function outputs the resolved mapping of each logical segment index
     * to its corresponding [device, row, column] LED coordinates, based on the internal
     * configuration (preset, custom map, or auto-mapped layout).
     *
     * Useful for verifying how segments are translated into physical driver outputs,
     * especially when using segment offsets, matrix presets, or split-device configurations.
     *
     * @param stream Reference to a `Stream` object (e.g., `Serial`) for printing output.
     *               Defaults to `Serial` if not specified.
     */
    void debugSegmentMapping(Stream &stream = Serial) { _barMeter.debugSegmentMapping(stream); }

    /**
     * @brief Set the on/off state of a logical segment (LED).
     *
     * @param segment Index of the logical segment to control (0 to `getSegNum() - 1`).
     * @param state   true to turn the LED on, false to turn it off.
     *
     * This function resolves the segment index to its corresponding [device, row, col]
     * location and updates the internal buffer of the underlying driver.
     * The change will not be visible until `show()` is called.
     */
    void setPixel(uint8_t segment, uint8_t state) { _barMeter.setPixel(segment, state); }

    /**
     * @brief Get the current state of a bar segment (pixel).
     *
     * This function returns the last known ON/OFF state of a segment.
     * It queries the driver’s internal buffer (not the physical IC).
     *
     * Direction and segment mapping are handled internally, so the returned
     * value always corresponds to the logical segment number.
     *
     * @param segment Index of the segment to query (0 to _segsNum - 1).
     * @return 1 if the segment is ON, 0 if OFF, or 0 if invalid or driver unavailable.
     */
    uint8_t getPixelState(uint8_t segment) { return _barMeter.getPixelState(segment); }

    /**
     * @brief Apply a segment-wise offset (shifts segment index before mapping to row/col).
     * @param offset Number of segments to skip.
     * @return Reference to this instance.
     *
     * This offset is better used with a 1 to 1 ICs output to led display
     * where the display do not start on the first IC output.
     *
     */
    SBK_BarDrive &setSegmentOffset(uint8_t offset)
    {
        _barMeter.setSegmentOffset(offset);
        return *this;
    }

    /**
     * @brief Apply a matrix-style physical offset for row/column positioning.
     * @param rowOffset Number of rows to shift downward.
     * @param colOffset Number of columns to shift rightward.
     * @return Reference to this instance.
     *
     *  Those offsets are better used with matrix led display matrixPreset, like BL28 matrixPreset
     *  where the matrix is not connected to the firsts IC row/column.
     *
     */
    SBK_BarDrive &setMatrixOffset(uint8_t rowOffset, uint8_t colOffset)
    {
        _barMeter.setMatrixOffset(rowOffset, colOffset);
        return *this;
    }

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
