/*---------------------------------------------------------*\
| RGBController_SkydimoSerial.cpp                           |
|                                                           |
|   RGBController for Skydimo Serial LED Strip              |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_SkydimoSerial.h"

/**
 * @brief Constructor.
 * @param controller_ptr A unique_ptr to the hardware controller.
 * @details Initializes controller information and modes.
 */
RGBController_SkydimoSerial::RGBController_SkydimoSerial(std::unique_ptr<SkydimoSerialController> controller_ptr)
    : controller(std::move(controller_ptr))
{
    // Set device information from the hardware controller
    name                = controller->GetDeviceName();
    vendor              = "Skydimo";
    type                = DEVICE_TYPE_LEDSTRIP;
    description         = "Skydimo Serial Device";
    version             = "1.0";
    serial              = controller->GetSerial();
    location            = controller->GetLocation();

    // Create Direct control mode
    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = 0;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    // Create Off mode
    mode Off;
    Off.name            = "Off";
    Off.value           = 1;
    Off.flags           = 0;
    Off.color_mode      = MODE_COLORS_NONE;
    modes.push_back(Off);

    // Create Stream mode (with keep-alive)
    mode Stream;
    Stream.name         = "Stream";
    Stream.value        = 2;
    Stream.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Stream.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Stream);

    SetupZones();
}

/**
 * @brief Destructor.
 * @details The unique_ptr will automatically handle hardware controller cleanup.
 */
RGBController_SkydimoSerial::~RGBController_SkydimoSerial()
{
}

/**
 * @brief Sets up the device zones.
 * @details Creates a single zone representing the LED strip with 100 LEDs.
 */
void RGBController_SkydimoSerial::SetupZones()
{
    // Create the LED strip zone
    zone strip_zone;
    strip_zone.name         = "LED Strip";
    strip_zone.type         = ZONE_TYPE_LINEAR;
    strip_zone.leds_min     = controller->GetLEDCount();
    strip_zone.leds_max     = controller->GetLEDCount();
    strip_zone.leds_count   = controller->GetLEDCount();
    strip_zone.matrix_map   = nullptr;
    zones.push_back(strip_zone);

    // Create LED objects for each LED
    for(int i = 0; i < controller->GetLEDCount(); i++)
    {
        led new_led;
        new_led.name = "LED " + std::to_string(i + 1);
        leds.push_back(new_led);
    }

    SetupColors();
}

/**
 * @brief Resizes a zone.
 * @param zone The index of the zone.
 * @param new_size The new size for the zone.
 * @note This device does not support resizing; the LED count is fixed at 100.
 */
void RGBController_SkydimoSerial::ResizeZone(int /*zone*/, int /*new_size*/)
{
    // This device does not support zone resizing.
}

/**
 * @brief Updates all LEDs on the device.
 * @details Sends the color array to the hardware.
 */
void RGBController_SkydimoSerial::DeviceUpdateLEDs()
{
    controller->SetLEDs(colors);
}

/**
 * @brief Updates the LEDs for a specific zone.
 * @param zone The index of the zone to update.
 * @details Since there is only one zone, this updates all LEDs.
 */
void RGBController_SkydimoSerial::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

/**
 * @brief Updates a single LED.
 * @param led The index of the LED to update.
 * @note This device requires a full update, so it calls DeviceUpdateLEDs.
 */
void RGBController_SkydimoSerial::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

/**
 * @brief Updates the device mode.
 * @details Starts or stops the keep-alive mechanism and turns off LEDs for the Off mode.
 */
void RGBController_SkydimoSerial::DeviceUpdateMode()
{
    // active_mode is a member of the base RGBController class.
    if (active_mode == 1) // Off mode
    {
        controller->StopKeepAlive();
        // Create an all-black color array to turn off LEDs.
        std::vector<RGBColor> black_colors(controller->GetLEDCount(), ToRGBColor(0, 0, 0));
        controller->SetLEDs(black_colors);
    }
    else if (active_mode == 2) // Stream mode
    {
        controller->StartKeepAlive();
    }
    else // Direct mode (active_mode == 0)
    {
        controller->StopKeepAlive();
    }
}
