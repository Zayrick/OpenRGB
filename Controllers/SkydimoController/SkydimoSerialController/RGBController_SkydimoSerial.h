/*---------------------------------------------------------*\
| RGBController_SkydimoSerial.h                             |
|                                                           |
|   RGBController for the Skydimo Serial LED Strip.         |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project.               |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SkydimoSerialController.h"
#include <memory>

/**
 * @brief OpenRGB controller class for the Skydimo Serial LED strip.
 * @details This class inherits from RGBController and implements the
 *          OpenRGB interface for the device.
 */
class RGBController_SkydimoSerial : public RGBController
{
public:
    explicit RGBController_SkydimoSerial(std::unique_ptr<SkydimoSerialController> controller_ptr);
    ~RGBController_SkydimoSerial();

    /**
     * @brief Sets up the device zones.
     * @details Creates a single zone representing the LED strip with 100 LEDs.
     */
    void SetupZones();

    /**
     * @brief Resizes a zone.
     * @param zone The index of the zone to resize.
     * @param new_size The new size for the zone.
     * @note This device does not support resizing zones.
     */
    void ResizeZone(int zone, int new_size);

    /**
     * @brief Updates all LEDs on the device.
     * @details Sends the colors of all LEDs to the device.
     */
    void DeviceUpdateLEDs();

    /**
     * @brief Updates the LEDs for a specific zone.
     * @param zone The index of the zone to update.
     */
    void UpdateZoneLEDs(int zone);

    /**
     * @brief Updates a single LED.
     * @param led The index of the LED to update.
     * @note This device requires a full update for all LEDs.
     */
    void UpdateSingleLED(int led);

    /**
     * @brief Updates the device mode.
     * @details This device only supports the "Direct" control mode.
     */
    void DeviceUpdateMode();

private:
    std::unique_ptr<SkydimoSerialController> controller;  ///< Pointer to the hardware controller
};
