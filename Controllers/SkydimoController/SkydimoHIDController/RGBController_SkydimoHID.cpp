/*---------------------------------------------------------*\
| RGBController_SkydimoHID.cpp                              |
|                                                           |
|   RGBController for Skydimo HID LED Strip                 |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_SkydimoHID.h"

/**
 * @brief Constructor.
 * @param controller_ptr A unique_ptr to the hardware controller.
 * @details Initializes controller information and modes.
 */
RGBController_SkydimoHID::RGBController_SkydimoHID(std::unique_ptr<SkydimoHIDController> controller_ptr)
    : controller(std::move(controller_ptr))
{
    // Set device information from the hardware controller
    name                = controller->GetDeviceName();
    vendor              = "Skydimo";
    type                = DEVICE_TYPE_LEDSTRIP;
    description         = "Skydimo HID LED Strip Controller";
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

    SetupZones();
}

/**
 * @brief Destructor.
 * @details The unique_ptr will automatically handle hardware controller cleanup.
 */
RGBController_SkydimoHID::~RGBController_SkydimoHID()
{
}

/**
 * @brief Sets up the device zones.
 * @details Creates a single zone representing the LED strip with a variable
 *          number of LEDs.
 */
void RGBController_SkydimoHID::SetupZones()
{
    // Clear existing zones and LEDs
    zones.clear();
    leds.clear();

    // Create the LED strip zone
    zone strip_zone;
    strip_zone.name         = "LED Strip";
    strip_zone.type         = ZONE_TYPE_LINEAR;
    strip_zone.leds_min     = 1;
    strip_zone.leds_max     = controller->GetMaxLEDCount();
    strip_zone.leds_count   = controller->GetMaxLEDCount(); // Default to max LEDs
    strip_zone.matrix_map   = nullptr;
    zones.push_back(strip_zone);

    // Create LED objects for each LED
    for(int i = 0; i < strip_zone.leds_count; i++)
    {
        led new_led;
        new_led.name = "LED " + std::to_string(i + 1);
        leds.push_back(new_led);
    }

    SetupColors();
}

/**
 * @brief Resizes a zone (the number of LEDs).
 * @param zone The index of the zone to resize (always 0).
 * @param new_size The new number of LEDs.
 * @details This device supports resizing the LED count.
 */
void RGBController_SkydimoHID::ResizeZone(int zone, int new_size)
{
    if(zone >= static_cast<int>(zones.size()))
    {
        return;
    }

    if(new_size < zones[zone].leds_min || new_size > zones[zone].leds_max)
    {
        return;
    }

    zones[zone].leds_count = new_size;

    // Re-create the LED list
    leds.clear();
    for(int i = 0; i < new_size; i++)
    {
        led new_led;
        new_led.name = "LED " + std::to_string(i + 1);
        leds.push_back(new_led);
    }

    SetupColors();
}

/**
 * @brief Updates all LEDs on the device.
 * @details Sends the color array to the hardware.
 */
void RGBController_SkydimoHID::DeviceUpdateLEDs()
{
    if(!zones.empty())
    {
        controller->SetLEDs(colors, zones[0].leds_count);
    }
}

/**
 * @brief Updates the LEDs for a specific zone.
 * @param zone The index of the zone to update.
 * @details Since there's only one zone, this just updates all LEDs.
 */
void RGBController_SkydimoHID::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

/**
 * @brief Updates a single LED.
 * @param led The index of the LED to update.
 * @note This device requires a full update, so it calls DeviceUpdateLEDs.
 */
void RGBController_SkydimoHID::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

/**
 * @brief Updates the device mode.
 * @details Handles mode changes: turns LEDs off for "Off" mode.
 */
void RGBController_SkydimoHID::DeviceUpdateMode()
{
    // active_mode is a member of the base RGBController class.
    if (active_mode == 1) // Off mode
    {
        // Create an all-black color array to turn off LEDs.
        if (!zones.empty())
        {
            std::vector<RGBColor> black_colors(zones[0].leds_count, ToRGBColor(0, 0, 0));
            controller->SetLEDs(black_colors, zones[0].leds_count);
        }
    }
    // "Direct" mode (active_mode == 0) requires no special handling here.
}
