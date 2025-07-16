/**
 * @file SBK_BarMeterAnimations.h
 * @brief Built-in animation engine for SBK_BarMeter-based LED displays.
 *
 * This file defines the `SBK_BarMeterAnimations` template class, which provides a rich set
 * of built-in animation sequences for LED bar meters. It supports dynamic animations like
 * filling, bouncing, signal tracking, block emissions, random pixel updates, and more.
 *
 * Compatible with any `SBK_BarMeter<DriverT>` instance, this class integrates directly with
 * SBK_BarDrive through its `.animations()` accessor.
 *
 * ### Highlights:
 * - Looping and one-shot animations
 * - Direction and logic inversion controls
 * - Live signal tracking and pointer rendering
 * - Block-based animations with emission control
 * - BPM-synchronized beat pulse effect
 * - Chainable animation starter functions
 * - Pixel state aware when `SBK_TRACK_PIXEL_STATE` is defined
 *
 * @tparam BarMeterT A specific instantiation of SBK_BarMeter (e.g., SBK_BarMeter<SBK_MAX72xxSoft>)
 *
 * @author
 * Samuel Barabé (Smart Builds & Kits)
 *
 * @version 2.0.0
 * @license MIT
 *
 * @copyright
 * Copyright (c) 2025 Samuel Barabé
 */

#pragma once

#include <Arduino.h>

/**
 * @class SBK_BarMeterAnimations
 * @brief Templated animation controller for SBK_BarMeter<T>.
 *
 * This class encapsulates timing logic, block emission, bouncing effects,
 * signal smoothing, and beat-based animations for bar meters. Chainable API
 * makes integration in update loops easy.
 *
 * @tparam BarMeterT A specific SBK_BarMeter<DriverT> instance
 */
template <typename BarMeterT>
class SBK_BarMeterAnimations
{
public:
    /**
     * @brief Construct an animation controller linked to a bar meter.
     * @param barMeter Reference to the associated SBK_BarMeter instance.
     */
    explicit SBK_BarMeterAnimations(BarMeterT &barMeter) : _barMeter(barMeter) {}

    /**
     * @brief Destructor. Frees block memory if allocated.
     */
    ~SBK_BarMeterAnimations()
    {
        if (_blocks)
        {
            delete[] _blocks;
            _blocks = nullptr;
        }
    }

    /**
     * @brief Set the total number of segments the animation logic should handle.
     * Typically called automatically from SBK_BarDrive.
     * @param n Number of segments in the bar.
     */
    void setSegsNum(uint8_t n)
    {
        _segsNum = n;
        _maxTracker = n - 1;
    }

    /**
     * @brief Advance the animation logic based on the current millis().
     * @param syncTime Optional timestamp to synchronize animation.
     * @return true if animation is still running; false otherwise.
     */
    bool update(uint32_t syncTime = millis())
    {
        _currentTime = syncTime;

        if (!_isRunning || _isPaused || !_currentFunc)
            return false;

        if ((this->*_currentFunc)())
        {
            if (_loop)
            {
                if (_skipPending)
                    _isLoopingNow = false;
                else
                {
                    _isLoopingNow = true; // mark it so user can react
                    _init = true;
                }
            }
            else
            {
                _isLoopingNow = false;
                _isRunning = false;
                _currentFunc = nullptr;
            }
        }
        return _isRunning;
    }

    /** @brief Mark animation as ready to reinitialize in next update cycle. */
    SBK_BarMeterAnimations &animInit()
    {
        _init = true;
        return *this;
    }

    /** @brief Pause animation progression. */
    SBK_BarMeterAnimations &pause()
    {
        _isPaused = true;
        return *this;
    }

    /** @brief Resume animation after pause. */
    SBK_BarMeterAnimations &resume()
    {
        _isPaused = false;
        return *this;
    }

    /** @brief Stop animation and reset internal pointers. */
    SBK_BarMeterAnimations &stop()
    {
        _isPaused = false;
        _isRunning = false;
        _skipPending = false;
        _currentFunc = nullptr;
        _animLogicSet = false;
        return *this;
    }

    /** @brief Set animation to automatically loop when complete. */
    SBK_BarMeterAnimations &loop()
    {
        _loop = true;
        return *this;
    }

    /** @brief Disable auto-looping. Animation will stop when complete. */
    SBK_BarMeterAnimations &noLoop()
    {
        _loop = false;
        return *this;
    }

    /** @brief Explicitly set the animation rendering direction. */
    SBK_BarMeterAnimations &setDir(bool newDirection)
    {
        _animRenderDirIsReversed = newDirection;
        _animDirSet = true;
        return *this;
    }

    /** @brief Toggle the current animation rendering direction. */
    SBK_BarMeterAnimations &toggleDir()
    {
        _animRenderDirIsReversed = !_animRenderDirIsReversed;
        _animDirSet = true;
        return *this;
    }

    /** @brief Reverse the rendering direction compared to the original one. */
    SBK_BarMeterAnimations &reverseDir()
    {
        _animRenderDirIsReversed = !_AnimInitDirIsReversed;
        _animDirSet = true;
        return *this;
    }

    /** @brief Reset direction to the original setting. */
    SBK_BarMeterAnimations &resetDir()
    {
        _animRenderDirIsReversed = _AnimInitDirIsReversed;
        _animDirSet = false;
        return *this;
    }

    /**
     * @brief Set rendering logic (true = inverted logic, false = normal logic), example fill logic become empty logic.
     * Logic refer to behavior, example : fill/empty, exploding/colliding, etc.
     */
    SBK_BarMeterAnimations &setLogic(bool newLogic)
    {
        if (_isNonInvertingLogicAnim)
            return *this;
        _animRenderLogicIsInverted = newLogic;
        _animLogicSet = true;
        return *this;
    }

    /**
     * @brief Toggle rendering logic (inverted vs normal), example fill logic become empty logic.
     * Logic refer to behavior, example : fill/empty, exploding/colliding, etc.
     * Some functions logic have no invertion, this function will have no effect on them.
     */
    SBK_BarMeterAnimations &toggleLogic()
    {
        if (_isNonInvertingLogicAnim)
            return *this;
        _animRenderLogicIsInverted = !_animRenderLogicIsInverted;
        _animLogicSet = true;
        return *this;
    }

    /**
     * @brief Reverse logic compared to initial state
     * Logic refer to behavior, example : fill/empty, exploding/colliding, etc.
     * Some functions logic have no invertion, this function will have no effect on them.
     */
    SBK_BarMeterAnimations &invertLogic()
    {
        if (_isNonInvertingLogicAnim)
            return *this;
        _animRenderLogicIsInverted = !_AnimInitLogicIsInverted;
        _animLogicSet = true;
        return *this;
    }

    /**
     * @brief Reset logic to initial (default) value.
     * Logic refer to behavior, example : fill/empty, exploding/colliding, etc.
     * Some functions logic have no invertion, this function will have no effect on them.
     */
    SBK_BarMeterAnimations &resetLogic()
    {
        if (_isNonInvertingLogicAnim)
            return *this;
        _animRenderLogicIsInverted = _AnimInitLogicIsInverted;
        _animLogicSet = false;
        return *this;
    }

    /** @brief Stop emitting new blocks during compatible block animations. */
    SBK_BarMeterAnimations &stopBlockEmission()
    {
        _emittingBlocksEnabled = false;
        return *this;
    }

    /** @brief Resume emitting new blocks during compatible block animations. */
    SBK_BarMeterAnimations &resumeBlockEmission()
    {
        _emittingBlocksEnabled = true;
        return *this;
    }

    /** @brief Query if animation is currently active. */
    bool isRunning() const { return _isRunning; }

    /** @brief Query if animation is paused. */
    bool isPaused() const { return _isPaused; }

    /** @brief Query if animation loop is enabled. */
    bool isLoopEnabled() const { return _loop; }

    /** @brief Detect whether animation has completed and is about to loop. */
    bool animPendingLoop()
    {
        if (_skipPending == true)
            _skipPending = false;
        else
        {
            if (_isLoopingNow)
            {
                _isLoopingNow = false; // auto-clear after read
                return true;
            }
        }
        return false;
    }

    /** @brief Returns true if current logic is different from original logic. */
    bool isLogicInverted()
    {
        if (_AnimInitLogicIsInverted != _animRenderLogicIsInverted)
            return true;
        else
            return false;
    }

    /** @brief Returns true if the current animation hasen't a logic invertion aka logic can't be inverted. */
    bool isNonInvertingLogicAnim() { return _isNonInvertingLogicAnim; }

    /** @brief Returns true if direction is reversed compared to the original. */
    bool isDirectionReversed()
    {
        if (_AnimInitDirIsReversed != _animRenderDirIsReversed)
            return true;
        else
            return false;
    }

    /** @brief Query if block emission is enabled. */
    bool isBlockEmissionEnabled() const { return _emittingBlocksEnabled; }

    // Animation starters
    /**
     * @brief Set all pixels to the specified state.
     * @param onoff Pixels desired state true = ON, false = off.
     */
    SBK_BarMeterAnimations &setAll(bool onff) { return onff ? setAllOn() : setAllOff(); }

    /**
     * @brief Set all pixels ON.
     */
    SBK_BarMeterAnimations &setAllOn()
    {
        _currentFunc = &SBK_BarMeterAnimations::_setAllOn;
        _isRunning = true;
        _init = true;
        return *this;
    }

    /**
     * @brief Set all pixels OFF.
     */
    SBK_BarMeterAnimations &setAllOff()
    {
        _currentFunc = &SBK_BarMeterAnimations::_setAllOff;
        _isRunning = true;
        _init = true;
        return *this;
    }

    /**
     * @brief Start animation that fills the bar upwards over a specified duration.
     * @param duration Total time in milliseconds.
     * @param maxPercent Top of the max fill range (0–100). Default is 100.
     * @param minPercent Bottom of the min fill range (0–100). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &fillUpDur(uint16_t duration, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _isRunning = true;
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        _updateIntv1 = max(5, duration / steps);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that fills the bar upwards at a fixed interval with live percent tracking.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercentPtr Pointer to maximum fill range percent value (0–100).
     * @param minPercentPtr Optional pointer to minimum fill range percent value (0–100). Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &fillUpIntv(uint16_t updateIntv, const uint8_t *maxPercentPtr, const uint8_t *minPercentPtr = nullptr)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _usePtr = true;
        _sigPtr2 = maxPercentPtr;
        _sigPtr1 = minPercentPtr;
        _mapMinMaxTrackerFromPtr(_sigPtr1, _sigPtr2, 0, _segsNum - 1);
        _isRunning = true;
        _updateIntv1 = max(5, updateIntv);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that fills the bar upwards at a fixed interval.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercent Top of the max fill range (0–100). Default is 100.
     * @param minPercent Bottom of the min fill range (0–100). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &fillUpIntv(uint16_t updateIntv, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _isRunning = true;
        _updateIntv1 = max(5, updateIntv);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that fills the bar downwards over a specified duration.
     * @param duration Total time in milliseconds.
     * @param maxPercent Top of the max fill range (0–100). Default is 100.
     * @param minPercent Bottom of the min fill range (0–100). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &fillDownDur(uint16_t duration, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _isRunning = true;
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        _updateIntv1 = max(5, duration / steps);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that fills the bar downwards at a fixed interval with live percent tracking.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercentPtr Pointer to maximum fill range percent value (0–100).
     * @param minPercentPtr Optional pointer to minimum fill range percent value (0–100). Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &fillDownIntv(uint16_t updateIntv, const uint8_t *maxPercentPtr, const uint8_t *minPercentPtr = nullptr)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = false;
        _usePtr = true;
        _sigPtr2 = maxPercentPtr;
        _sigPtr1 = minPercentPtr;
        _mapMinMaxTrackerFromPtr(_sigPtr1, _sigPtr2, 0, _segsNum - 1);
        _isRunning = true;
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        _updateIntv1 = max(5, updateIntv);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that fills the bar downwards at a fixed interval.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercent Top of the max fill range (0–100). Default is 100.
     * @param minPercent Bottom of the min fill range (0–100). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &fillDownIntv(uint16_t updateIntv, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _isRunning = true;
        _updateIntv1 = max(5, updateIntv);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that empties the bar from top to bottom over a duration.
     * @param duration Total time in milliseconds.
     * @param maxPercent Top of the max fill range (0–100). Default is 100.
     * @param minPercent Bottom of the min fill range (0–100). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &emptyDownDur(uint16_t duration, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = true;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _isRunning = true;
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        return emptyDownIntv(duration / steps, maxPercent, minPercent);
    }

    /**
     * @brief Start animation that empties the bar from top to bottom at fixed intervals with live range.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercentPtr Pointer to maximum fill range percent value (0–100).
     * @param minPercentPtr Optional pointer to minimum fill range percent value (0–100). Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &emptyDownIntv(uint16_t updateIntv, const uint8_t *maxPercentPtr, const uint8_t *minPercentPtr = nullptr)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = true;
        _usePtr = true;
        _sigPtr2 = maxPercentPtr;
        _sigPtr1 = minPercentPtr;
        _mapMinMaxTrackerFromPtr(_sigPtr1, _sigPtr2, 0, _segsNum - 1);
        _isRunning = true;
        _updateIntv1 = max(5, updateIntv);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that empties the bar from top to bottom at fixed intervals.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercent Top of the max fill range (0–100). Default is 100.
     * @param minPercent Bottom of the min fill range (0–100). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &emptyDownIntv(uint16_t updateIntv, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = true;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _isRunning = true;
        _updateIntv1 = max(5, updateIntv);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that empties the bar from bottom to top over a duration.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercentPtr Pointer to top max fill range percent value.
     * @param minPercentPtr Pointer to bottom min fill range percent value (optional). Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &emptyUpDur(uint16_t duration, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = true;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _isRunning = true;
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        _updateIntv1 = max(5, duration / steps);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that empties the bar from bottom to top at fixed intervals with live range.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercentPtr Pointer to top max fill range percent value.
     * @param minPercentPtr Pointer to bottom min fill range percent value (optional). Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &emptyUpIntv(uint16_t updateIntv, const uint8_t *maxPercentPtr, const uint8_t *minPercentPtr = nullptr)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = true;
        _usePtr = true;
        _sigPtr2 = maxPercentPtr;
        _sigPtr1 = minPercentPtr;
        _mapMinMaxTrackerFromPtr(_sigPtr1, _sigPtr2, 0, _segsNum - 1);
        _isRunning = true;
        _updateIntv1 = max(5, updateIntv);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that empties the bar from bottom to top at fixed intervals.
     * @param updateIntv Time between updates in milliseconds.
     * @param maxPercent Top of the max fill range (0–100). Default is 100.
     * @param minPercent Bottom of the min fill range (0–100). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &emptyUpIntv(uint16_t updateIntv, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = true;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _isRunning = true;
        _updateIntv1 = max(5, updateIntv);
        _currentFunc = &SBK_BarMeterAnimations::_fillOrEmpty;
        return *this;
    }

    /**
     * @brief Start animation that fills/empties (bounces) bar upward with fixed range.
     * @param duration Total time in milliseconds for full bounce cycle.
     * @param maxPercent Maximum fill percentage (0–100). Default is 100.
     * @param minPercent Minimum fill percentage (0–100). Default is 0.
     * @param fillIntv Optional fixed interval for the fill phase. Default is 0 (auto).
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillUpDur(uint16_t duration, uint8_t maxPercent = 100, uint8_t minPercent = 0, uint16_t fillIntv = 0)
    {
        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        uint16_t emptyIntv = fillIntv;
        if (fillIntv == 0)
        {
            fillIntv = round(max(5, duration) / (2 * steps));
            emptyIntv = fillIntv;
        }
        else
        {
            emptyIntv = round(max(5, duration - fillIntv * steps) / (steps));
        }
        _updateIntv1 = fillIntv;     // Starting interval
        _updateIntv2 = _updateIntv1; // Fill intv
        _updateIntv3 = emptyIntv;    // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFill;
        return *this;
    }

    /**
     * @brief Start animation that fills/empties (bounces) bar upward with live range.
     * @param fillIntv Fill interval in ms.
     * @param emptyIntv Empty interval in ms.
     * @param maxPercentPtr Pointer to maximum percentage fill range value.
     * @param minPercentPtr Pointer to minimum percentage fill range value (optional). Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillUpIntv(uint16_t fillIntv, uint16_t emptyIntv, const uint8_t *maxPercentPtr, const uint8_t *minPercentPtr = nullptr)
    {
        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = false;
        _sigPtr2 = maxPercentPtr;
        _sigPtr1 = minPercentPtr;
        _usePtr = true;
        _mapMinMaxTrackerFromPtr(_sigPtr1, _sigPtr2, 0, _segsNum - 1);
        _updateIntv1 = max(5, fillIntv);  // Starting interval
        _updateIntv2 = _updateIntv1;      // Fill intv
        _updateIntv3 = max(5, emptyIntv); // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFill;
        return *this;
    }

    /**
     * @brief Start animation that fills/empties (bounces) bar upward with fixed range.
     * @param fillIntv Fill interval in ms. Default is 10.
     * @param emptyIntv Empty interval in ms. Default is 10.
     * @param maxPercent Max fill percent fill range. Default is 100.
     * @param minPercent Min fill percent fill range. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillUpIntv(uint16_t fillIntv = 10, uint16_t emptyIntv = 10, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _updateIntv1 = max(5, fillIntv);  // Starting interval
        _updateIntv2 = _updateIntv1;      // Fill intv
        _updateIntv3 = max(5, emptyIntv); // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFill;
        return *this;
    }

    /**
     * @brief Start animation that fills/empties (bounces) bar downward.
     * @param duration Total time in milliseconds for full bounce cycle.
     * @param maxPercent Maximum fill percentage fill range. Default is 100.
     * @param minPercent Minimum fill percentage fill range. Default is 0.
     * @param fillIntv Optional interval override for the fill phase. Default is 0 (auto).
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillDownDur(uint16_t duration, uint8_t maxPercent = 100, uint8_t minPercent = 0, uint16_t fillIntv = 0)
    {
        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        uint16_t emptyIntv = fillIntv;
        if (fillIntv == 0)
        {
            fillIntv = round(max(5, duration) / (2 * steps));
            emptyIntv = fillIntv;
        }
        else
        {
            emptyIntv = round(max(5, duration - fillIntv * steps) / (steps));
        }
        _updateIntv1 = fillIntv;     // Starting interval
        _updateIntv2 = _updateIntv1; // Fill intv
        _updateIntv3 = emptyIntv;    // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFill;
        return *this;
    }

    /**
     * @brief Start downward fill/empty (bounce) with live percent range tracking.
     * @param fillIntv Fill interval in ms.
     * @param emptyIntv Empty interval in ms.
     * @param maxPercentPtr Pointer to max percent fill range.
     * @param minPercentPtr Pointer to min percent fill range. Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillDownIntv(uint16_t fillIntv, uint16_t emptyIntv, const uint8_t *maxPercentPtr, const uint8_t *minPercentPtr = nullptr)
    {
        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = false;
        _sigPtr2 = maxPercentPtr;
        _sigPtr1 = minPercentPtr;
        _usePtr = true;
        _mapMinMaxTrackerFromPtr(_sigPtr1, _sigPtr2, 0, _segsNum - 1);
        _updateIntv1 = max(5, fillIntv);  // Starting interval
        _updateIntv2 = _updateIntv1;      // Fill intv
        _updateIntv3 = max(5, emptyIntv); // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFill;
        return *this;
    }

    /**
     * @brief Start downwardfill/empty (bounce) with fixed percent range.
     * @param fillIntv Fill interval in ms. Default is 10.
     * @param emptyIntv Empty interval in ms. Default is 10.
     * @param maxPercent Maximum fill percent. Default is 100.
     * @param minPercent Minimum fill percent. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillDownIntv(uint16_t fillIntv = 10, uint16_t emptyIntv = 10, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, 0, _segsNum - 1);
        _updateIntv1 = max(5, fillIntv);  // Starting interval
        _updateIntv2 = _updateIntv1;      // Fill intv
        _updateIntv3 = max(5, emptyIntv); // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFill;
        return *this;
    }

    /**
     * @brief Bounce fill from center outward with fixed range.
     * @param duration Total time in milliseconds for full bounce cycle.
     * @param maxPercent Maximum fill percentage fill range. Default is 100.
     * @param minPercent Minimum fill percentage fill range. Default is 0.
     * @param fillIntv Optional interval override for the fill phase. Default is 0 (auto).
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillFromCenterDur(uint16_t duration, uint8_t maxPercent = 100, uint8_t minPercent = 0, uint16_t fillIntv = 0)
    {
        _isNonInvertingLogicAnim = true;
        _AnimInitLogicIsInverted = false;
        _animRenderDirIsReversed = false;
        _mirrorHalfRangeDir = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, (_segsNum / 2) - 1, 0);
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        uint16_t emptyIntv = fillIntv;
        if (fillIntv == 0)
        {
            fillIntv = round(max(5, duration) / (2 * steps));
            emptyIntv = fillIntv;
        }
        else
        {
            emptyIntv = round(max(5, duration - fillIntv * steps) / (steps));
        }
        _updateIntv1 = fillIntv;     // Starting interval
        _updateIntv2 = _updateIntv1; // Fill intv
        _updateIntv3 = emptyIntv;    // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFillHalfRangeMirrorCenter;
        return *this;
    }

    /**
     * @brief Bounce fill from center outward with live percent range tracking.
     * @param fillIntv Fill interval in ms.
     * @param emptyIntv Empty interval in ms.
     * @param maxPercentPtr Pointer to max percent fill range.
     * @param minPercentPtr Pointer to min percent fill range. Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillFromCenterIntv(uint16_t fillIntv, uint16_t emptyIntv, const uint8_t *maxPercentPtr, const uint8_t *minPercentPtr = nullptr)
    {
        _isNonInvertingLogicAnim = true;
        _AnimInitLogicIsInverted = false;
        _animRenderDirIsReversed = false;
        _mirrorHalfRangeDir = false;
        _sigPtr2 = maxPercentPtr;
        _sigPtr1 = minPercentPtr;
        _usePtr = true;
        _mapMinMaxTrackerFromPtr(minPercentPtr, maxPercentPtr, (_segsNum / 2) - 1, 0);
        _updateIntv1 = fillIntv;     // Starting interval
        _updateIntv2 = _updateIntv1; // Fill intv
        _updateIntv3 = emptyIntv;    // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFillHalfRangeMirrorCenter;
        return *this;
    }

    /**
     * @brief Bounce fill from center outward with fixed range.
     * @param fillIntv Fill interval in ms. Default is 25.
     * @param emptyIntv Empty interval in ms. Default is 25.
     * @param maxPercent Maximum range percent to animate. Default is 100.
     * @param minPercent Minimum range percent to animate. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillFromCenterIntv(uint16_t fillIntv = 25, uint16_t emptyIntv = 25, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = true;
        _AnimInitLogicIsInverted = false;
        _animRenderDirIsReversed = false;
        _mirrorHalfRangeDir = false;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, (_segsNum / 2) - 1, 0);
        _updateIntv1 = fillIntv;     // Starting interval
        _updateIntv2 = _updateIntv1; // Fill intv
        _updateIntv3 = emptyIntv;    // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFillHalfRangeMirrorCenter;
        return *this;
    }

    /**
     * @brief Bounce fill from both edges inward with fixed range.
     * @param duration Total time in milliseconds for full bounce cycle.
     * @param maxPercent Maximum fill percentage fill range. Default is 100.
     * @param minPercent Minimum fill percentage fill range. Default is 0.
     * @param fillIntv Optional interval override for the fill phase. Default is 0 (auto).
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillFromEdgesDur(uint16_t duration, uint8_t maxPercent = 100, uint8_t minPercent = 0, uint16_t fillIntv = 0)
    {
        _isNonInvertingLogicAnim = true;
        _AnimInitLogicIsInverted = false;
        _animRenderDirIsReversed = false;
        _mirrorHalfRangeDir = true;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, (_segsNum / 2) - 1, 0);
        uint8_t steps = max(1, _maxTracker - _minTracker + 1);
        uint16_t emptyIntv = fillIntv;
        if (fillIntv == 0)
        {
            fillIntv = round(max(5, duration) / (2 * steps));
            emptyIntv = fillIntv;
        }
        else
        {
            emptyIntv = round(max(5, duration - fillIntv * steps) / (steps));
        }
        _updateIntv1 = fillIntv;     // Starting interval
        _updateIntv2 = _updateIntv1; // Fill intv
        _updateIntv3 = emptyIntv;    // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFillHalfRangeMirrorCenter;
        return *this;
    }

    /**
     * @brief Bounce fill from both edges inward with live percent range tracking..
     * @param fillIntv Fill interval in ms.
     * @param emptyIntv Empty interval in ms.
     * @param maxPercentPtr Pointer to max percent fill range.
     * @param minPercentPtr Pointer to min percent fill range. Default is nullptr.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillFromEdgesIntv(uint16_t fillIntv, uint16_t emptyIntv, const uint8_t *maxPercentPtr, const uint8_t *minPercentPtr = nullptr)
    {
        _isNonInvertingLogicAnim = true;
        _AnimInitLogicIsInverted = false;
        _animRenderDirIsReversed = false;
        _mirrorHalfRangeDir = true;
        _sigPtr2 = maxPercentPtr;
        _sigPtr1 = minPercentPtr;
        _usePtr = true;
        _mapMinMaxTrackerFromPtr(minPercentPtr, maxPercentPtr, (_segsNum / 2) - 1, 0);
        _updateIntv1 = fillIntv;     // Starting interval
        _updateIntv2 = _updateIntv1; // Fill intv
        _updateIntv3 = emptyIntv;    // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFillHalfRangeMirrorCenter;
        return *this;
    }

    /**
     * @brief Bounce fill from both edges inward with fix range.
     * @param fillIntv Fill interval in ms. Default is 25.
     * @param emptyIntv Empty interval in ms. Default is 25.
     * @param maxPercent Maximum range percent to animate. Default is 100.
     * @param minPercent Minimum range percent to animate. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &bounceFillFromEdgesIntv(uint16_t fillIntv = 25, uint16_t emptyIntv = 25, uint8_t maxPercent = 100, uint8_t minPercent = 0)
    {
        _isNonInvertingLogicAnim = true;
        _AnimInitLogicIsInverted = false;
        _animRenderDirIsReversed = false;
        _mirrorHalfRangeDir = true;
        _usePtr = false;
        _sigPtr2 = nullptr;
        _sigPtr1 = nullptr;
        _mapMinMaxTracker(minPercent, maxPercent, (_segsNum / 2) - 1, 0);
        _updateIntv1 = fillIntv;     // Starting interval
        _updateIntv2 = _updateIntv1; // Fill intv
        _updateIntv3 = emptyIntv;    // Empty intv
        _sequenceState = 0;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_bounceFillHalfRangeMirrorCenter;
        return *this;
    }

    /**
     * @brief Start a beat pulse animation synchronized to a live BPM value.
     * @param bpmPtr Pointer to a variable holding the BPM value.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &beatPulse(uint8_t *bpmPtr) // bouncing from bottom (maybe like a volume meter with the music)
    {
        _sigPtr1 = bpmPtr;
        _param2 = min(35 * (_segsNum - 1) / 100, 255); // MIN_BASE_LEVEL
        _param2 = min(67 * (_segsNum - 1) / 100, 255); // MIN_PEAK_LEVEL
        _param4 = 150;                                     // PEAK_HOLD_TIME
        _usePtr = true;
        _isNonInvertingLogicAnim = true;
        _AnimInitLogicIsInverted = false;
        _animRenderDirIsReversed = false;

        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_beatPulse;
        _init - true;
        return *this;
    }

    /**
     * @brief Start a beat pulse animation synchronized to a constant BPM.
     * @param bpm Beats per minute. Default is 116.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &beatPulse(uint8_t bpm = 116) // bouncing from bottom (maybe like a volume meter with the music)
    {
        _param1 = max(1, bpm);
        _param2 = min(35 * (_segsNum - 1) / 100, 255); // MIN_BASE_LEVEL
        _param3 = min(67 * (_segsNum - 1) / 100, 255); // MIN_PEAK_LEVEL
        _param4 = 150;                                     // PEAK_HOLD_TIME
        _sigPtr1 = nullptr;
        _usePtr = false;
        _isNonInvertingLogicAnim = true;
        _AnimInitLogicIsInverted = false;
        _animRenderDirIsReversed = false;

        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_beatPulse;
        _init = true;
        return *this;
    }

    /**
     * @brief Emit mirrored blocks from center outward.
     * @param intv Step interval in milliseconds. Default is 50.
     * @param blockLength Number of pixels per block. Default is 2.
     * @param blockSpacing Space between blocks. Default is 1.
     * @param numBlocks Number of blocks to emit (0 = infinite). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &explodingBlocks(uint16_t intv = 50, uint8_t blockLength = 2, uint8_t blockSpacing = 1, uint8_t numBlocks = 0)
    {
        _updateIntv1 = max(5, intv);
        _param1 = blockLength;
        _param2 = blockSpacing;
        _param3 = numBlocks;
        _param4 = ((_segsNum / 2) / (blockLength + blockSpacing)) + 2;
        _param4 = constrain(_param4, 2, 32);

        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = true;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_mirrorBlocks;
        return *this;
    }

    /**
     * @brief Emit mirrored blocks from edge to center.
     * @param intv Step interval in milliseconds. Default is 50.
     * @param blockLength Number of pixels per block. Default is 2.
     * @param blockSpacing Space between blocks. Default is 1.
     * @param numBlocks Number of blocks to emit (0 = infinite). Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &collidingBlocks(uint16_t intv = 50, uint8_t blockLength = 2, uint8_t blockSpacing = 1, uint8_t numBlocks = 0)
    {

        _updateIntv1 = max(5, intv);
        _param1 = blockLength;
        _param2 = blockSpacing;
        _param3 = numBlocks;
        _param4 = ((_segsNum / 2) / (blockLength + blockSpacing)) + 2;
        _param4 = constrain(_param4, 2, 32);

        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_mirrorBlocks;
        return *this;
    }

    /**
     * @brief Scroll blocks upward.
     * @param intv Update interval in ms. Default is 50.
     * @param blockLength Block length in segments. Default is 2.
     * @param blockSpacing Space between blocks. Default is 1.
     * @param numBlocks Number of blocks to show. 0 = infinite. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &scrollingUpBlocks(uint16_t intv = 50, uint8_t blockLength = 2, uint8_t blockSpacing = 1, uint8_t numBlocks = 0)
    {
        _updateIntv1 = max(5, intv);
        _param1 = blockLength;
        _param2 = blockSpacing;
        _param3 = numBlocks;
        _param4 = (_segsNum / (blockLength + blockSpacing)) + 2;
        _param4 = constrain(_param4, 2, 64);

        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_scrollingBlocks;
        return *this;
    }

    /**
     * @brief Scroll blocks downward.
     * @param intv Update interval in ms. Default is 50.
     * @param blockLength Block length in segments. Default is 2.
     * @param blockSpacing Space between blocks. Default is 1.
     * @param numBlocks Number of blocks to show. 0 = infinite. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &scrollingDownBlocks(uint16_t intv = 50, uint8_t blockLength = 2, uint8_t blockSpacing = 1, uint8_t numBlocks = 0)
    {
        _updateIntv1 = max(5, intv);
        _param1 = blockLength;
        _param2 = blockSpacing;
        _param3 = numBlocks;
        _param4 = (_segsNum / (blockLength + blockSpacing)) + 2;
        _param4 = constrain(_param4, 2, 64);

        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = true;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_scrollingBlocks;
        return *this;
    }

    /**
     * @brief Drop blocks from top and stack from bottom up.
     * @param intv Update interval in milliseconds. Default is 50.
     * @param blockLength Number of segments per block. Default is 1.
     * @param blockSpacing Space between blocks. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &downStackingBlocks(uint16_t intv = 50, uint8_t blockLength = 1, uint8_t blockSpacing = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _updateIntv1 = max(5, intv);
        _param1 = blockLength;
        _param2 = blockSpacing;
        _param3 = 0; // requested number of blocks
        _param4 = 1; // aka maximum blocks number
        if (_init)
            _emittingBlocksEnabled = true;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_stackingBlocks;
        return *this;
    }

    /**
     * @brief Launch blocks upward and unstack from top.
     * @param intv Update interval in milliseconds. Default is 50.
     * @param blockLength Number of segments per block. Default is 1.
     * @param blockSpacing Space between blocks. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &upUnstackingBlocks(uint16_t intv = 50, uint8_t blockLength = 1, uint8_t blockSpacing = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = true;
        _updateIntv1 = max(5, intv);
        _param1 = blockLength;
        _param2 = blockSpacing;
        _param3 = 0; // aka requested number of blocks
        _param4 = 1; // aka maximum blocks number
        if (_init)
            _emittingBlocksEnabled = true;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_stackingBlocks;
        return *this;
    }

    /**
     * @brief Launch blocks from bottom and stack at the top.
     * @param intv Update interval in milliseconds. Default is 50.
     * @param blockLength Number of segments per block. Default is 1.
     * @param blockSpacing Space between blocks. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &upStackingBlocks(uint16_t intv = 50, uint8_t blockLength = 1, uint8_t blockSpacing = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = false;
        _updateIntv1 = max(5, intv);
        _param1 = blockLength;
        _param2 = blockSpacing;
        _param3 = 0; // aka requested number of blocks
        _param4 = 1; // aka maximum blocks number
        if (_init)
            _emittingBlocksEnabled = true;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_stackingBlocks;
        return *this;
    }

    /**
     * @brief Drop blocks from top and unstack from bottom up.
     * @param intv Update interval in milliseconds. Default is 50.
     * @param blockLength Number of segments per block. Default is 1.
     * @param blockSpacing Space between blocks. Default is 0.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &downUnstackingBlocks(uint16_t intv = 50, uint8_t blockLength = 1, uint8_t blockSpacing = 0)
    {
        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = true;
        _updateIntv1 = max(5, intv);
        _param1 = blockLength;
        _param2 = blockSpacing;
        _param3 = 0; // aka requested number of blocks
        _param4 = 1; // aka maximum blocks number
        if (_init)
            _emittingBlocksEnabled = true;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_stackingBlocks;
        return *this;
    }

    /**
     * @brief Follow analog signal with smoothing for fill animation.
     * @param sigPtr Pointer to analog signal.
     * @param updateIntv Update interval in milliseconds. Default is 100.
     * @param minMap Minimum input value. Default is 0.
     * @param maxMap Maximum input value. Default is 1023.
     * @param smoothingFactor Smoothing strength 0–100. Default is 30.
     * @param samplingIntv signal sampling rate. Default is 5.
     * @return Reference to this animation instance (chainable).
     */
    SBK_BarMeterAnimations &followSignalSmooth(const uint16_t *sigPtr, uint16_t updateIntv = 100, uint16_t minMap = 0, uint16_t maxMap = 1023, uint8_t smoothingFactor = 30, uint16_t samplingIntv = 5)
    {

        _sigPtr1 = sigPtr;
        _updateIntv1 = max(10, updateIntv);
        _minMap = max(0, minMap);
        _maxMap = max(0, maxMap);
        _correctSwapOrEqualMinMax(_minMap, _maxMap);
        _param1 = constrain(smoothingFactor, 0, 100);
        _updateIntv2 = samplingIntv;

        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _isRunning = true;
        _init = true;
        _currentFunc = &SBK_BarMeterAnimations::_followSignalSmooth;
        return *this;
    }

    /**
     * @brief Follow signal with smoothing and display a pointer.
     * @param sigPtr Pointer to analog signal.
     * @param updateIntv Update interval in milliseconds. Default is 100.
     * @param minMap Minimum input value. Default is 0.
     * @param maxMap Maximum input value. Default is 1023.
     * @param smoothingFactor Smoothing strength 0–100. Default is 30.
     * @param samplingIntv signal sampling rate. Default is 5.
     * @return Reference to this animation instance (chainable).
     */
    SBK_BarMeterAnimations &followSignalWithPointer(const uint16_t *sigPtr, uint16_t updateIntv = 100, uint16_t minMap = 0, uint16_t maxMap = 1023, uint8_t smoothingFactor = 30, uint16_t samplingIntv = 5)
    {
        _sigPtr1 = sigPtr;
        _updateIntv1 = max(10, updateIntv);
        _minMap = max(0, minMap);
        _maxMap = max(0, maxMap);
        _correctSwapOrEqualMinMax(_minMap, _maxMap);
        _param1 = constrain(smoothingFactor, 0, 100);
        _updateIntv2 = samplingIntv;

        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _isRunning = true;
        _init = true;
        _currentFunc = &SBK_BarMeterAnimations::_followSignalWithPointer;
        return *this;
    }

    /**
     * @brief Smoothly fills the lower half and upper half of the bar meter independently from center,
     * based on two signal levels.
     * @param sigPtr1 Pointer to analog signal1 for bottom half (center to bottom)
     * @param updateIntv Update interval in milliseconds. Default is 100.
     * @param sig2Ptr          Pointer to signal2 for top half (center to top). Default nullptr, signal 1 will be mirrored.
     * @param minMap           Minimum raw value to map. Default 0.
     * @param maxMap           Maximum raw value to map. Default 1023.
     * @param smoothingFactor Smoothing strength 0–100. Default is 30.
     * @param samplingIntv signal sampling rate. Default is 5.
     * @return Reference to this animation instance (chainable).
     */
    SBK_BarMeterAnimations &followDualSignalFromCenter(const uint16_t *sig1Ptr, uint16_t updateIntv = 100, const uint16_t *sig2Ptr = nullptr, uint16_t minMap = 0, uint16_t maxMap = 1023, uint8_t smoothingFactor = 30, uint16_t samplingIntv = 5)
    {
        _sigPtr1 = sig1Ptr;
        _updateIntv1 = max(10, updateIntv);
        _sigPtr2 = sig2Ptr ? sig2Ptr : sig1Ptr;
        _minMap = max(0, minMap);
        _maxMap = max(0, maxMap);
        _correctSwapOrEqualMinMax(_minMap, _maxMap);
        _param1 = constrain(smoothingFactor, 0, 100);
        _updateIntv2 = samplingIntv;

        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        //_mirrorHalfRangeDir = false;
        _isRunning = true;
        _init = true;
        _currentFunc = &SBK_BarMeterAnimations::_followDualSignalCenterMirror;
        return *this;
    }

    /**
     * @brief Smoothly fills the lower half and upper half of the bar meter independently from center,
     * based on two signal levels.
     * @param sigPtr1 Pointer to analog signal1 for bottom half (bottom edge to center)
     * @param updateIntv Update interval in milliseconds. Default is 100.
     * @param sig2Ptr          Pointer to signal2 for top half (top edge to center). Default nullptr, signal 1 will be mirrored.
     * @param updateIntv       Update interval in milliseconds.
     * @param sig1Ptr          Pointer to signal level for bottom half (center to bottom).
     * @param sig2Ptr          Pointer to signal level for top half (center to top). Default nullptr.
     * @param minMap           Minimum raw value to map. Default 0.
     * @param maxMap           Maximum raw value to map. Default 1023.
     * @param smoothingFactor Smoothing strength 0–100. Default is 30.
     * @param samplingIntv signal sampling rate. Default is 5.
     * @return Reference to this animation instance (chainable).
     */
    SBK_BarMeterAnimations &followDualSignalFromEdges(const uint16_t *sig1Ptr, uint16_t updateIntv = 100, const uint16_t *sig2Ptr = nullptr, uint16_t minMap = 0, uint16_t maxMap = 1023, uint8_t smoothingFactor = 30, uint16_t samplingIntv = 5)
    {
        _sigPtr1 = sig1Ptr;
        _updateIntv1 = max(10, updateIntv);
        _sigPtr2 = sig2Ptr ? sig2Ptr : sig1Ptr;
        _minMap = max(0, minMap);
        _maxMap = max(0, maxMap);
        _correctSwapOrEqualMinMax(_minMap, _maxMap);
        _param1 = constrain(smoothingFactor, 0, 100);
        _updateIntv2 = samplingIntv;

        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = true;
        _AnimInitLogicIsInverted = false;
        //_mirrorHalfRangeDir = true;
        _isRunning = true;
        _init = true;
        _currentFunc = &SBK_BarMeterAnimations::_followDualSignalCenterMirror;
        return *this;
    }

    /**
     * @brief Follow an analog signal with smoothing and floating peak indicator.
     *
     * This animation fills the bar based on a smoothed analog input value and displays a peak marker
     * that holds momentarily before decaying.
     *
     * @param sigPtr Pointer to the analog signal value (e.g., from analogRead()).
     * @param peakHoldTime Duration (in ms) to hold the peak before it begins to decay.
     * @param updateIntv Bar update interval in milliseconds. Default is 100 ms.
     * @param minMap Minimum expected raw signal value. Default is 0.
     * @param maxMap Maximum expected raw signal value. Default is 1023.
     * @param smoothingFactor Signal smoothing strength (0–100). Higher = smoother. Default is 30.
     * @param samplingIntv Signal sampling interval in milliseconds. Default is 5 ms.
     * @return Reference to this animation instance (chainable).
     */
    SBK_BarMeterAnimations &followSignalFloatingPeak(const uint16_t *sigPtr, uint8_t peakHoldTime = 20, uint16_t updateIntv = 100, uint16_t minMap = 0, uint16_t maxMap = 1023, uint8_t smoothingFactor = 30, uint16_t samplingIntv = 5)
    {

        _sigPtr1 = sigPtr;
        _updateIntv3 = max(20, peakHoldTime);
        _updateIntv1 = max(10, updateIntv);
        _minMap = max(0, minMap);
        _maxMap = max(0, maxMap);
        _correctSwapOrEqualMinMax(_minMap, _maxMap);
        _param1 = constrain(smoothingFactor, 0, 100);
        _updateIntv2 = samplingIntv;

        _isNonInvertingLogicAnim = true;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _isRunning = true;
        _init = true;
        _currentFunc = &SBK_BarMeterAnimations::_followSignalFloatingPeak;
        return *this;
    }

    /**
     * @brief Randomly light up pixels until the bar is full.
     * @param interval Update interval in milliseconds. Default is 30.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &randomFill(uint16_t interval = 30)
    {
        _updateIntv1 = interval;

        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = false;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_randomPixelUpdater;
        _init = true;
        return *this;
    }

    /**
     * @brief Randomly turn off pixels until the bar is empty.
     * @param interval Update interval in milliseconds. Default is 30.
     * @return Reference to this animation instance.
     */
    SBK_BarMeterAnimations &randomEmpty(uint16_t interval = 30)
    {
        _updateIntv1 = interval;

        _isNonInvertingLogicAnim = false;
        _animRenderDirIsReversed = false;
        _AnimInitLogicIsInverted = true;
        _isRunning = true;
        _currentFunc = &SBK_BarMeterAnimations::_randomPixelUpdater;
        _init = true;
        return *this;
    }

protected:
    BarMeterT &_barMeter;
    uint8_t _segsNum = 0;

    // Active animation update function
    using AnimUpdateFn = bool (SBK_BarMeterAnimations::*)();
    AnimUpdateFn _currentFunc = nullptr;

    // Control flags
    bool _init = true; // true = init function
    bool _isRunning = false;
    bool _isPaused = false;
    bool _loop = false;
    bool _isLoopingNow = false;
    bool _AnimInitLogicIsInverted = false;
    bool _animRenderLogicIsInverted = false;
    bool _prevAnimRenderLogic = false;
    bool _isNonInvertingLogicAnim = false;
    bool _mirrorHalfRangeDir = false;
    bool _animLogicSet = false;
    bool _skipPending = false;
    bool _animRenderDirIsReversed = false;
    bool _AnimInitDirIsReversed = false;
    bool _animDirSet = false;
    bool _usePtr = false;
    bool _emittingBlocksEnabled = true;

    // Time trackings
    uint32_t _currentTime = 0;
    uint32_t _lastUpdate1 = 0, _lastUpdate2 = 0, _lastUpdate3 = 0;

    // Animations update interval trackers
    uint16_t _updateIntv1 = 10, _updateIntv2 = 10, _updateIntv3 = 10;

    // Animations trackers
    uint8_t _sequenceState = 0;
    int8_t _ledTracker1 = 0, _ledTracker2 = 0, _ledTracker3 = 0;
    int8_t _minTracker = 0, _maxTracker = 1;
    uint8_t _param1 = 0, _param2 = 0, _param3 = 0, _param4 = 0, _param5 = 0;
    uint16_t _smoothedValue1 = 0, _smoothedValue2 = 0;
    uint16_t _minMap = 0, _maxMap = 1023;
    uint8_t _counter1 = 0, _counter2 = 0;
    // Live signals trackers
    const uint16_t *_sigPtr1 = nullptr, *_sigPtr2 = nullptr;
    // Blocks related helpers
    struct Block
    {
        Block() : position(-1), active(false) {}
        int8_t position;
        bool active;
    };
    Block *_blocks = nullptr;

    // Animation helpers
    static inline void _normalizePercentRange(uint8_t &minP, uint8_t &maxP)
    {
        minP = constrain(minP, 0, 100);
        maxP = constrain(maxP, 0, 100);
        if (minP > maxP)
        {
            uint8_t t = maxP;
            maxP = minP;
            minP = t;
        }
        if (minP == maxP)
        {
            if (maxP < 100)
                maxP = minP + 1;
            else
                minP--;
        }
    }
    inline void _mapMinMaxTrackerFromPtr(uint16_t *minPercPtr, uint16_t *maxPercPtr, int8_t minR, uint8_t maxR)
    {
        if (!_usePtr)
            return;
        uint8_t minP = minPercPtr ? *minPercPtr : 0;
        uint8_t maxP = maxPercPtr ? *maxPercPtr : 100;
        _normalizePercentRange(minP, maxP);
        _minTracker = map(minP, 0, 100, minR, maxR);
        _maxTracker = map(maxP, 0, 100, minR, maxR);
    }
    inline void _mapMinMaxTracker(uint8_t minP, uint8_t maxP, uint8_t minR, uint8_t maxR)
    {
        _normalizePercentRange(minP, maxP);
        _minTracker = map(minP, 0, 100, minR, maxR);
        _maxTracker = map(maxP, 0, 100, minR, maxR);
    }
    inline uint8_t _corrPixelToDir(uint8_t pixel)
    {
        return _animRenderDirIsReversed ? (_segsNum - 1) - pixel : pixel;
    }
    inline uint8_t _corrPixelToDirForHalfRange(uint8_t pixel)
    {
        if (!_mirrorHalfRangeDir)
            return pixel;

        const uint8_t half = _segsNum / 2;
        return abs((half - 1) - pixel);
    }
    static inline void _correctSwapOrEqualMinMax(uint16_t &minVal, uint16_t &maxVal)
    {
        if (minVal > maxVal)
        {
            uint16_t temp = maxVal;
            maxVal = minVal;
            minVal = temp;
        }
        if (minVal == maxVal) // avoid division by zero in map()
            if (maxVal < 65, 535)
                maxVal = minVal + 1;
            else
                minVal--;
    }
    static inline uint8_t _getMappedSignal(uint16_t sig, uint16_t minM, uint16_t maxM, uint8_t minR, uint8_t maxR)
    {
        uint8_t mapped = map(sig, minM, maxM, minR, maxR);
        return constrain(mapped, minR, maxR); // avoid edge overshoot
    }

    // Animation update functions

    bool _setAllOn()
    {
        _init = false;
        for (uint8_t i = 0; i < _segsNum; i++)
            _barMeter.setPixel(i, true);
        return true;
    }
    bool _setAllOff()
    {
        _init = false;
        _barMeter.clear();
        return true;
    }

    bool _fillOrEmpty()
    {
        _mapMinMaxTrackerFromPtr(_sigPtr1, _sigPtr2, 0, _segsNum - 1);

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            // Pre-fill for emptyDown mode
            if (_animRenderLogicIsInverted)
            {
                _ledTracker1 = _maxTracker;
                for (uint8_t i = 0; i < _segsNum; ++i)
                {
                    if (i <= _maxTracker)
                        _barMeter.setPixel(_corrPixelToDir(i), true);
                    else
                        _barMeter.setPixel(_corrPixelToDir(i), false);
                }
            }
            else // Pre-fill fillUp, light up to _minTracker inclusively
            {
                _ledTracker1 = _minTracker;
                for (uint8_t i = 0; i < _segsNum; ++i)
                {
                    if (i <= _minTracker)
                        _barMeter.setPixel(_corrPixelToDir(i), true);
                    else
                        _barMeter.setPixel(_corrPixelToDir(i), false);
                }
            }
            return false;
        }

        if (_animRenderLogicIsInverted != _prevAnimRenderLogic)
        {
            _prevAnimRenderLogic = _animRenderLogicIsInverted;

            //  Clamp if inverted logic and tracker is beyond maxTracker
            if (_animRenderLogicIsInverted && _ledTracker1 > _maxTracker)
                _ledTracker1 = _maxTracker;

            // Clamp in normal logic mode
            if (!_animRenderLogicIsInverted && _ledTracker1 < _minTracker)
                _ledTracker1 = _minTracker;
        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;

            if (_animRenderLogicIsInverted)
            {
                if (_ledTracker1 >= _minTracker && _ledTracker1 >= _minTracker)
                {
                    _barMeter.setPixel(_corrPixelToDir(_ledTracker1), false);
                    _ledTracker1--;
                }
                else
                    return true; // now done!
            }
            else
            {
                if (_ledTracker1 <= _maxTracker && _ledTracker1 < _segsNum)
                {
                    _barMeter.setPixel(_corrPixelToDir(_ledTracker1), true);
                    _ledTracker1++;
                }
                else
                    return true; // done lighting up all
            }
        }
        return false;
    }

    bool _bounceFill()
    {
        switch (_sequenceState)
        {
        case 0: // Fill Up

            _skipPending = false;
            if (_fillOrEmpty())
            {
                _animRenderLogicIsInverted = !_AnimInitLogicIsInverted;
                _sequenceState = 1;
                _updateIntv1 = _updateIntv3; // Empty update interval
            }
            return false;

        case 1: // Fill Down

            _skipPending = true;
            if (_fillOrEmpty())
            {
                _animRenderLogicIsInverted = _AnimInitLogicIsInverted;
                _sequenceState = 0;
                _updateIntv1 = _updateIntv2; // Fill up  update interval
                return true;
            }
            return false;
        }

        return true; // Shouldn’t reach here
    }

    bool _fillFromOrEmptyToCenter()
    {
        const uint8_t center = _segsNum / 2;
        _mapMinMaxTrackerFromPtr(_sigPtr1, _sigPtr2, center - 1, 0);

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            if (_animRenderLogicIsInverted)
            {
                _ledTracker1 = _maxTracker;

                // Start fully lit
                for (uint8_t i = 0; i < center; ++i)
                {
                    if (i >= _maxTracker)
                    {
                        _barMeter.setPixel(_corrPixelToDirForHalfRange(i), true);
                        _barMeter.setPixel((_segsNum - 1) - _corrPixelToDirForHalfRange(i), true);
                    }
                    else
                    {
                        _barMeter.setPixel(_corrPixelToDirForHalfRange(i), false);
                        _barMeter.setPixel((_segsNum - 1) - _corrPixelToDirForHalfRange(i), false);
                    }
                }
            }
            else
            {

                _ledTracker1 = _minTracker;

                // Start with minimum fill
                for (uint8_t i = 0; i < center; ++i)
                {
                    if (i > _minTracker)
                    {
                        _barMeter.setPixel(_corrPixelToDirForHalfRange(i), true);
                        _barMeter.setPixel((_segsNum - 1) - _corrPixelToDirForHalfRange(i), true);
                    }
                    else
                    {
                        _barMeter.setPixel(_corrPixelToDirForHalfRange(i), false);
                        _barMeter.setPixel((_segsNum - 1) - _corrPixelToDirForHalfRange(i), false);
                    }
                }
            }
            return false;
        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;
            if (!_animRenderLogicIsInverted)
            {
                if (_ledTracker1 >= _maxTracker && _ledTracker1 >= 0)
                {
                    _barMeter.setPixel(_corrPixelToDirForHalfRange(_ledTracker1), true);
                    _barMeter.setPixel((_segsNum - 1) - _corrPixelToDirForHalfRange(_ledTracker1), true);
                    _ledTracker1--;
                }
                else
                    return true; // finished filling
            }
            else
            {
                if (_ledTracker1 <= _minTracker && _ledTracker1 < center)
                {
                    _barMeter.setPixel(_corrPixelToDirForHalfRange(_ledTracker1), false);
                    _barMeter.setPixel((_segsNum - 1) - _corrPixelToDirForHalfRange(_ledTracker1), false);
                    _ledTracker1++;
                }
                else
                    return true; // finished clearing
            }
        }

        return false;
    }

    bool _bounceFillHalfRangeMirrorCenter()
    {
        switch (_sequenceState)
        {
        case 0: // Fill from center

            _skipPending = true;
            if (_fillFromOrEmptyToCenter())
            {
                _animRenderLogicIsInverted = !_AnimInitLogicIsInverted;
                _updateIntv1 = _updateIntv3; // Empty update interval
                _sequenceState = 1;
            }
            return false;

        case 1: // Empty to center

            _skipPending = false;
            if (_fillFromOrEmptyToCenter())
            {
                _animRenderLogicIsInverted = _AnimInitLogicIsInverted;
                _updateIntv1 = _updateIntv2; // Empty update interval
                _sequenceState = 0;
                return true; // one full bounce cycle completed
            }
            return false;
        }

        return true; // should not happen
    }

#define bpm _param1
#define MIN_BASE_LEVEL _param2
#define MIN_PEAK_LEVEL _param3
#define PEAK_HOLD_TIME _param4
#define bpmPtr _sigPtr1
#define currentLevel _ledTracker1
#define peakLevel _ledTracker2
#define beat _updateIntv1
#define lastBeat _lastUpdate1
#define lastRandomUpdate _lastUpdate2
#define prevPeakUpdate _lastUpdate3
    bool _beatPulse() // bouncing from bottom (maybe like a volume meter with the music)
    {
        static int8_t randomOffset;
        static bool isPeak = false;

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            _setAllOff();
            currentLevel = MIN_BASE_LEVEL;
            peakLevel = MIN_PEAK_LEVEL;
        }

        // Get bpm from pointer if needed
        if (_usePtr)
        {
            bpm = bpmPtr ? max(1, *bpmPtr) : 116;
        }
        beat = 60000 / bpm;

        // Update at bpm pulse
        if (_currentTime - lastBeat >= beat)
        {
            lastBeat = _currentTime;
            isPeak = !isPeak; // Toggle between peak and base level
        }

        // Smooth transition between base and peak levels
        if (isPeak && currentLevel <= MIN_PEAK_LEVEL)
        {
            currentLevel += random(3, 5); // Gradual increase
        }
        else if (!isPeak && currentLevel >= MIN_BASE_LEVEL)
        {
            currentLevel -= random(0, 4); // Gradual decrease
        }

        // Randomized additional LEDs on top at a random interval
        if (_currentTime - lastRandomUpdate >= uint16_t(random(50, 300)))
        {
            lastRandomUpdate = _currentTime;
            randomOffset = random(-4, 4); // 0 to 5 extra LEDs
        }

        uint8_t finalLevel = currentLevel + randomOffset;
        finalLevel = constrain(finalLevel, 0, _segsNum);
        if (finalLevel > _segsNum)
            finalLevel = _segsNum;
        if (finalLevel < 0)
            finalLevel = 0;

        // Update peak level
        if (finalLevel > peakLevel)
        {
            peakLevel = min(finalLevel, _segsNum - 1);
            prevPeakUpdate = _currentTime; // Reset decay timer
        }
        else if (_currentTime - prevPeakUpdate >= PEAK_HOLD_TIME && peakLevel > finalLevel)
        {
            peakLevel--;                   // Slowly drop peak
            prevPeakUpdate = _currentTime; // Reset decay timer
        }

        // Update LED states
        for (uint8_t i = 0; i < _segsNum; i++)
        {
            _barMeter.setPixel(_corrPixelToDir(i), (i < finalLevel));
        }

        // Ensure peak LED stays on
        if (peakLevel < _segsNum)
        {
            _barMeter.setPixel(_corrPixelToDir(peakLevel), true);
        }
        return false; // Continuous pulse animation, never auto-terminates
    }
#undef bpm
#undef MIN_BASE_LEVEL
#undef MIN_PEAK_LEVEL
#undef PEAK_HOLD_TIME
#undef bpmPtr
#undef currentLevel
#undef peakLevel
#undef beat
#undef lastBeat
#undef lastRandomUpdate
#undef prevPeakUpdate

#define blockLength _param1
#define blockSpacing _param2
#define requestedNumBlocks _param3
#define maxBlocks _param4
#define emitIndex _param5
#define emittedBlocksCount _counter1
#define emitCooldown _counter2
    void _emitBlock(int8_t pos)
    {
        if (!_blocks || maxBlocks < 1)
            return;

        if (emitCooldown > 0)
        {
            emitCooldown--;
            return;
        }

        if (requestedNumBlocks > 0 && emittedBlocksCount >= requestedNumBlocks)
            return;

        for (uint8_t i = 0; i < maxBlocks; ++i)
        {
            uint8_t idx = (emitIndex + i) % maxBlocks;
            Block &b = _blocks[idx];

            if (!b.active)
            {
                b.position = pos;
                b.active = true;
                emittedBlocksCount++;

                emitCooldown = blockLength + blockSpacing - 1;
                emitIndex = (emitIndex + 1) % maxBlocks;
                return;
            }
        }
    }

    static inline int8_t _calculateSwtichPostion(int8_t pos, uint8_t bockL, uint8_t range)
    {
        // Compute switched block head position
        return (range - 1) - pos + (bockL - 1);
    }

    int8_t _calculateSwitchedEmitTickCounter(uint8_t range)
    {
        const int8_t emitInterval = blockLength + blockSpacing;

        int8_t closestSwp = -127; // Track the nearest visible block from new side

        for (uint8_t i = 0; i < maxBlocks; ++i)
        {
            Block &b = _blocks[i];

            if (!b.active)
                continue;

            int8_t p = b.position; // head position before switching

            // Step 1: compute switched block head position
            int8_t swp = _calculateSwtichPostion(p, blockLength, range);

            b.position = swp;

            // Step 2: if out of visible range, deactivate block
            if (swp < 0)
            {
                b.active = false;
                continue;
            }

            // // Step 3: Keep the first (closest to emit side) visible block
            if (closestSwp == -127 || swp < closestSwp)
                closestSwp = swp;
        }

        if (closestSwp >= 0)
            return (emitInterval - 1) - closestSwp;
        else
            return 0; // fallback if no visible block found
    }

    bool _mirrorBlocks()
    {
        const uint8_t center = _segsNum / 2;

        if (_init)
        {
            _init = false;
            _emittingBlocksEnabled = true;

            emittedBlocksCount = 0;
            emitCooldown = 0;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            if (_blocks)
            {
                delete[] _blocks;
                _blocks = nullptr;
            }
            _blocks = new Block[maxBlocks]{};
            return false;
        }

        if (_prevAnimRenderLogic != _animRenderLogicIsInverted)
        {
            emitCooldown = _calculateSwitchedEmitTickCounter(_segsNum / 2);
            emittedBlocksCount = requestedNumBlocks ;
            _prevAnimRenderLogic = _animRenderLogicIsInverted;
        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;
            // _frameCounter++;
            _barMeter.clear();

            if ((requestedNumBlocks == 0 || emittedBlocksCount < requestedNumBlocks) && _emittingBlocksEnabled)
                _emitBlock(-1); // Because the new block will move to 0 postion in first iteration

            for (uint8_t i = 0; i < maxBlocks; ++i)
            {
                Block &b = _blocks[i];
                if (!b.active)
                    continue;

                b.position++;

                // Only render to lower half (0 to _centerTracker - 1)
                uint16_t clamped = max(0, (int16_t)b.position + 1);
                uint8_t pixelsVisible = min(blockLength, (uint8_t)clamped);
                for (uint8_t j = 0; j < pixelsVisible; ++j)
                {
                    int8_t pos = _animRenderLogicIsInverted ? center - 1 - b.position : b.position;
                    int8_t tailOffset = _animRenderLogicIsInverted ? pos + j : pos - j;

                    if (tailOffset < 0)
                        continue;

                    int8_t idx = tailOffset;
                    if (idx >= center)
                        continue;

                    int8_t mirrorIdx = _segsNum - 1 - idx;

                    if (idx >= 0 && idx < _segsNum)
                    {
                        _barMeter.setPixel(idx, true);
                    }
                    if (mirrorIdx != idx && mirrorIdx >= 0 && mirrorIdx < _segsNum)
                    {
                        _barMeter.setPixel(mirrorIdx, true);
                    }
                }

                // Deactivate block if head has passed the visual range
                if (b.position >= center - 1 + blockLength)
                    b.active = false;
            }
        }
        // animation as ended if emmit is disable and all blocks are cleared
        if ((requestedNumBlocks > 0 && emittedBlocksCount >= requestedNumBlocks) || !_emittingBlocksEnabled)
        {
            bool anyActive = false;
            for (uint8_t i = 0; i < maxBlocks; ++i)
            {
                if (_blocks[i].active)
                {
                    anyActive = true;
                    break;
                }
            }
            if (!anyActive)
                return true; // animation complete
        }

        return false;
    }

    bool _scrollingBlocks()
    {
        if (_init)
        {
            _init = false;
            _emittingBlocksEnabled = true;

            emittedBlocksCount = 0;
            emitCooldown = 0;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            if (_blocks)
            {
                delete[] _blocks;
                _blocks = nullptr;
            }
            _blocks = new Block[maxBlocks]{};
            return false;
        }

        if (_prevAnimRenderLogic != _animRenderLogicIsInverted)
        {
            emitCooldown = _calculateSwitchedEmitTickCounter(_segsNum);
            emittedBlocksCount = requestedNumBlocks ;
            _prevAnimRenderLogic = _animRenderLogicIsInverted;

        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;
            _setAllOff();

            if ((requestedNumBlocks == 0 || emittedBlocksCount < requestedNumBlocks) && _emittingBlocksEnabled)
                _emitBlock(-1); // Start fully off-screen left

            for (uint8_t i = 0; i < maxBlocks; ++i)
            {
                Block &b = _blocks[i];
                if (!b.active)
                    continue;

                b.position++;

                for (uint8_t j = 0; j < blockLength; ++j)
                {
                    int8_t pos = _animRenderLogicIsInverted ? (_segsNum - 1) - b.position : b.position;
                    int8_t tailOffset = _animRenderLogicIsInverted ? pos + j : pos - j;

                    if (tailOffset < 0)
                        continue;

                    int8_t idx = tailOffset;
                    if (idx >= _segsNum)
                        continue;

                    if (idx >= 0 && idx < _segsNum)
                        _barMeter.setPixel(_corrPixelToDir(idx), true);
                }

                // Deactivate block if head has passed the visual range
                if (b.position >= (_segsNum - 1) + blockLength)
                    b.active = false;
            }
        }

        if ((requestedNumBlocks > 0 && emittedBlocksCount >= requestedNumBlocks) || !_emittingBlocksEnabled)
        {
            for (uint8_t i = 0; i < maxBlocks; ++i)
            {
                if (_blocks[i].active)
                    return false;
            }
            return true;
        }

        return false;
    }

#define stackLevel _ledTracker1
    bool _stackingBlocks()
    {
        const uint8_t blockInterval = blockLength + blockSpacing;
        bool hasActive = false; // active block tracker

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            maxBlocks = 1;
            if (_blocks)
            {
                delete[] _blocks;
                _blocks = nullptr;
            }
            _blocks = new Block[maxBlocks]{};

            emitIndex = 0;
            emitCooldown = 0;

            stackLevel = 0;
            if (!_animRenderLogicIsInverted)
            {
                _barMeter.clear();
                // Falling blocks
                // stackLevel = 0;
            }
            else
            {
                // Flying upward - fill pattern first
                while (stackLevel < _segsNum)
                    stackLevel += blockInterval;
                for (uint8_t i = 0; i < stackLevel; i++)
                {
                    if ((i % blockInterval) < blockLength)
                        _barMeter.setPixel(_corrPixelToDir(i), true);
                    else
                        _barMeter.setPixel(_corrPixelToDir(i), false);
                }
            }

            return false;
        }

        if (_animRenderLogicIsInverted != _prevAnimRenderLogic)
        {
            _prevAnimRenderLogic = _animRenderLogicIsInverted;
            if (_animRenderLogicIsInverted)
                stackLevel += blockInterval;
            else
                stackLevel -= blockInterval;
        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;

            // Only clear pixels of active blocks, not the whole bar
            for (uint8_t i = 0; i < maxBlocks; ++i)
            {
                Block &b = _blocks[i];
                if (!b.active)
                    continue;

                for (uint8_t j = 0; j < blockLength; ++j)
                {
                    int8_t seg = b.position + j;
                    if (seg >= 0 && seg < _segsNum)
                        _barMeter.setPixel(_corrPixelToDir(seg), false);
                }
            }

            // Emit one block if none active
            for (uint8_t i = 0; i < maxBlocks; ++i)
            {
                if (_blocks[i].active)
                {
                    hasActive = true;
                    break;
                }
            }
            if (!hasActive)
            {
                emitCooldown = 0; // emit blocks on demand
                if (!_animRenderLogicIsInverted && stackLevel <= _segsNum)
                    _emitBlock(_segsNum); // fall from top
                else if (_animRenderLogicIsInverted && stackLevel >= 0)
                    _emitBlock(stackLevel - blockInterval);
            }

            // Update and draw all active blocks
            for (uint8_t i = 0; i < maxBlocks; ++i)
            {
                Block &b = _blocks[i];
                if (!b.active)
                    continue;

                // Clear trailing pixel of previous frame
                int8_t clearPos = b.position;
                if (clearPos >= 0 && clearPos < _segsNum)
                    _barMeter.setPixel(_corrPixelToDir(clearPos), false);

                // Move block
                if (!_animRenderLogicIsInverted)
                    b.position--;
                else
                    b.position++;

                for (uint8_t j = 0; j < blockLength; ++j)
                {
                    int8_t seg = b.position + j;
                    if (seg >= 0 && seg < _segsNum)
                        _barMeter.setPixel(_corrPixelToDir(seg), true);
                }

                if (!_animRenderLogicIsInverted)
                {
                    if (b.position <= stackLevel)
                    {
                        stackLevel += blockInterval;
                        b.active = false;
                    }
                }
                else
                {
                    if (b.position >= _segsNum)
                    {
                        stackLevel -= blockInterval;
                        b.active = false;
                    }
                }
            }

            // Draw base for stacking
            if (stackLevel == 0)
            {
                _barMeter.setPixel(_corrPixelToDir(0), false);
            }
            if (!_animRenderLogicIsInverted)
            {
                for (int8_t i = 0; i < stackLevel - blockInterval; ++i)
                {
                    if ((i % blockInterval) < blockLength)
                        _barMeter.setPixel(_corrPixelToDir(i), true);
                    else
                        _barMeter.setPixel(_corrPixelToDir(i), false);
                }
            }
            else
            {
                for (uint8_t i = 0; i < stackLevel - blockInterval; i++)
                {
                    if ((i % blockInterval) < blockLength)
                        _barMeter.setPixel(_corrPixelToDir(i), true);
                    else
                        _barMeter.setPixel(_corrPixelToDir(i), false);
                }
            }
            if (!_animRenderLogicIsInverted)
            {
                if (stackLevel >= _segsNum - 1 && !hasActive)
                    return true;
            }
            else
            {
                if (stackLevel <= 0 && !hasActive)
                    return true;
            }
        }
        return false;
    }
#undef stackLevel
#undef blockLength
#undef blockSpacing
#undef requestedNumBlocks
#undef maxBlocks
#undef emitIndex
#undef emittedBlocksCount
#undef emitCooldown

#define smoothingFactor _param1
    bool _followSignalSmooth()
    {
        if (!_sigPtr1)
        {
            _setAllOff();
            return true;
        }

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            _smoothedValue1 = *_sigPtr1;
            _lastUpdate1 = _currentTime;
            _setAllOff();
            return false;
        }

        // Signal sampling
        if (_currentTime - _lastUpdate2 >= _updateIntv2)
        {
            uint16_t raw = *_sigPtr1;
            _smoothedValue1 = (raw * smoothingFactor + _smoothedValue1 * (100 - smoothingFactor)) / 100;
            _lastUpdate2 = _currentTime;
        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;

            uint8_t level = _getMappedSignal(_smoothedValue1, _minMap, _maxMap, 0, _segsNum);

            for (uint8_t i = 0; i < _segsNum; i++)
            {
                _barMeter.setPixel(_corrPixelToDir(i), i < level);
            }
        }
        return false; // continuous animation
    }
    bool _followSignalWithPointer()
    {
        if (!_sigPtr1)
        {
            _setAllOff();
            return true;
        }

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            _smoothedValue1 = *_sigPtr1;
            _lastUpdate1 = _currentTime;
            _setAllOff();
            return false;
        }

        // Signal sampling
        if (_currentTime - _lastUpdate2 >= _updateIntv2)
        {
            uint16_t raw = *_sigPtr1;
            _smoothedValue1 = (raw * smoothingFactor + _smoothedValue1 * (100 - smoothingFactor)) / 100;
            _lastUpdate2 = _currentTime;
        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;

            uint8_t avg = _getMappedSignal(_smoothedValue1, _minMap, _maxMap, 0, _segsNum);
            uint8_t pointer = _getMappedSignal(*_sigPtr1, _minMap, _maxMap, 0, _segsNum);

            // First, fill up to moving average
            for (uint8_t i = 0; i < _segsNum; i++)
            {
                _barMeter.setPixel(_corrPixelToDir(i), i < avg);
            }

            // Then selectively clear area around pointer
            if (pointer < avg && pointer > 0)
                _barMeter.setPixel(_corrPixelToDir(pointer - 1), false); // clear just below if pointer is inside the fill

            if (pointer < avg - 2)
                _barMeter.setPixel(_corrPixelToDir(pointer + 1), false); // clear just above

            // Finally, show the pointer
            _barMeter.setPixel(_corrPixelToDir(pointer), true);
        }
        return false; // continuous animation
    }
    bool _followDualSignalCenterMirror()
    {
        if (!_sigPtr1 && !_sigPtr2)
        {
            _setAllOff();
            return true;
        }

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _mirrorHalfRangeDir = _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            _smoothedValue1 = *_sigPtr1;
            if (_sigPtr2)
                _smoothedValue2 = *_sigPtr2;
            _lastUpdate1 = _currentTime;
            _setAllOff();
            return false;
        }

        if (_animRenderLogicIsInverted != _prevAnimRenderLogic)
            _prevAnimRenderLogic = _animRenderLogicIsInverted;

        // Signals sampling
        if (_currentTime - _lastUpdate2 >= _updateIntv2)
        {
            _lastUpdate2 = _currentTime;

            uint16_t raw1 = *_sigPtr1;
            _smoothedValue1 = (uint16_t)(((uint32_t)smoothingFactor * raw1 + (uint32_t)(100 - smoothingFactor) * _smoothedValue1) / 100);

            if (_sigPtr2)
            {
                uint16_t raw2 = *_sigPtr2;
                _smoothedValue2 = (uint16_t)(((uint32_t)smoothingFactor * raw2 + (uint32_t)(100 - smoothingFactor) * _smoothedValue2) / 100);
            }
        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;

            uint16_t raw1 = *_sigPtr1;
            _smoothedValue1 = (uint16_t)(((uint32_t)smoothingFactor * raw1 + (uint32_t)(100 - smoothingFactor) * _smoothedValue1) / 100);
            uint8_t level1 = _getMappedSignal(_smoothedValue1, _minMap, _maxMap, 0, _segsNum / 2);

            uint8_t level2 = level1;
            if (_sigPtr2)
                level2 = _getMappedSignal(_smoothedValue2, _minMap, _maxMap, 0, _segsNum / 2);

            for (uint8_t i = 0; i < _segsNum; i++)
            {
                //_barMeter.setPixel(_corrPixelToDirForHalfRange(i), i >= (_segsNum / 2 - 1) - level1 && i <= ((_segsNum / 2) + level2));
                if (_animRenderLogicIsInverted)
                    _barMeter.setPixel(i, i < (_segsNum / 2 - 1) - level1 || i > ((_segsNum / 2) + level2));
                else
                    _barMeter.setPixel(i, i >= (_segsNum / 2 - 1) - level1 && i <= ((_segsNum / 2) + level2));
            }
        }
        return false; // continuous animation
    }

#define currentLevel _ledTracker1
#define peakLevel _ledTracker3
#define baseUpdate _updateIntv1
#define lastBaseUpdate _lastUpdate1
#define peakHoldTime _updateIntv3
#define lastPeakUpdate _lastUpdate3
    bool _followSignalFloatingPeak() // bouncing from bottom (maybe like a volume meter with the music)
    {

        if (!_sigPtr1)
        {
            _setAllOff();
            return true;
        }

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            _setAllOff();
            _smoothedValue1 = *_sigPtr1;
            lastBaseUpdate = _currentTime;
            lastPeakUpdate = _currentTime;
            currentLevel = 0;
            peakLevel = 0;
            return false;
        }

        // Signal smoothing update
        if (_currentTime - _lastUpdate2 >= _updateIntv2)
        {
            _lastUpdate2 = _currentTime;

            uint16_t raw = *_sigPtr1;
            _smoothedValue1 = (raw * smoothingFactor + _smoothedValue1 * (100 - smoothingFactor)) / 100;
            currentLevel = _getMappedSignal(_smoothedValue1, _minMap, _maxMap, 0, _segsNum);
        }

        // Pixel + peak update
        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;

            // Peak logic
            if (currentLevel > peakLevel)
            {
                peakLevel = min(currentLevel, _segsNum - 1);
                lastPeakUpdate = _currentTime; // Reset decay timer
            }
            else if (_currentTime - lastPeakUpdate >= peakHoldTime && peakLevel > currentLevel)
            {
                peakLevel--;                   // Slowly drop peak
                lastPeakUpdate = _currentTime; // Reset decay timer
            }

            // Draw bar
            for (uint8_t i = 0; i < _segsNum; i++)
            {
                _barMeter.setPixel(_corrPixelToDir(i), (i <= currentLevel));
            }

            // Draw peak
            if (peakLevel < _segsNum)
            {
                _barMeter.setPixel(_corrPixelToDir(peakLevel), true);
            }
        }
        return false; // Continuous pulse animation, never auto-terminates
    }
#undef currentLevel
#undef peakLevel
#undef baseUpdate
#undef lastBaseUpdate
#undef peakHoldTime
#undef lastPeakUpdate

#undef smoothingFactor

#define cursor _ledTracker1
    bool _randomPixelUpdater()
    {

        static uint8_t *pixelOrder = nullptr;

        if (_init)
        {
            _init = false;

            if (!_animLogicSet)
                _prevAnimRenderLogic = _animRenderLogicIsInverted = _AnimInitLogicIsInverted;

            if (_animRenderLogicIsInverted)
                _setAllOn();
            else
                _setAllOff();

            if (pixelOrder)
            {
                delete[] pixelOrder;
                pixelOrder = nullptr;
            }

            pixelOrder = new uint8_t[_segsNum];
            for (uint8_t i = 0; i < _segsNum; ++i)
                pixelOrder[i] = i;

            for (uint8_t i = _segsNum - 1; i > 0; --i)
            {
                uint8_t j = random(0, i + 1);
                uint8_t tmp = pixelOrder[i];
                pixelOrder[i] = pixelOrder[j];
                pixelOrder[j] = tmp;
            }

            cursor = 0;
            _lastUpdate1 = _currentTime;

            return false;
        }

        if (_currentTime - _lastUpdate1 >= _updateIntv1)
        {
            _lastUpdate1 = _currentTime;

            uint8_t retries = 0;
            while (cursor < _segsNum && retries++ < _segsNum - 1)
            {
                uint8_t seg = pixelOrder[cursor];
                bool currentState = _barMeter.getPixelState(seg);
                bool shouldChange = (!_AnimInitLogicIsInverted && !currentState) || (_AnimInitLogicIsInverted && currentState);

                if (shouldChange)
                {
                    _barMeter.setPixel(seg, !_AnimInitLogicIsInverted);
                    cursor++;
                    break; // Change only one pixel per interval
                }
                else
                {
                    cursor++; // Move on to next even if it doesn't need change
                }
            }
        }
        if (cursor >= _segsNum)
        {
            delete[] pixelOrder;
            pixelOrder = nullptr;
            return true;
        }
        return false;
    }
#undef cursor
};
