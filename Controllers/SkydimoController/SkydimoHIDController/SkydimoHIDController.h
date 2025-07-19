/*---------------------------------------------------------*\
| SkydimoHIDController.h                                    |
|                                                           |
|   Driver for the Skydimo HID LED Strip.                   |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project.               |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include "RGBController.h"

struct hid_device_;

/**
 * @brief Controller class for the Skydimo HID LED strip.
 * @details This class handles communication with the LED strip over HID,
 *          supporting a variable number of LEDs.
 */
class SkydimoHIDController
{
public:
    SkydimoHIDController();
    ~SkydimoHIDController();

    /**
     * @brief Initializes the controller and opens the HID device.
     * @param path The system path of the HID device.
     * @return true on success, false on failure.
     */
    bool Initialize(const std::string& path);

    /**
     * @brief Closes the HID device.
     */
    void CloseDevice();

    /**
     * @brief Sets the colors for the LEDs.
     * @param colors A vector of RGBColor structs.
     * @param count The number of LEDs to update.
     * @return true on success, false on failure.
     */
    bool SetLEDs(const std::vector<RGBColor>& colors, int count);

    /**
     * @brief Gets the device name.
     * @return A string containing the device name.
     */
    std::string GetDeviceName() const { return device_name; }

    /**
     * @brief Gets the device serial number.
     * @return A string containing the serial number.
     */
    std::string GetSerial() const { return device_serial; }

    /**
     * @brief Gets the device location (system path).
     * @return A string containing the device path.
     */
    std::string GetLocation() const { return device_path; }

    /**
     * @brief Gets the Vendor ID (VID).
     * @return The 16-bit Vendor ID.
     */
    uint16_t GetVendorID() const { return vid; }

    /**
     * @brief Gets the Product ID (PID).
     * @return The 16-bit Product ID.
     */
    uint16_t GetProductID() const { return pid; }

    /**
     * @brief Gets the maximum supported number of LEDs.
     * @return The maximum LED count.
     */
    int GetMaxLEDCount() const { return max_leds; }

private:
    static constexpr uint16_t VID = 0x1A86;           ///< Vendor ID
    static constexpr uint16_t PID = 0xE316;           ///< Product ID
    static constexpr int MAX_RGB_BYTES = 60;          ///< Max RGB bytes per HID report
    static constexpr int BATCH_LEDS = 20;             ///< Number of LEDs to process per batch
    static constexpr int DEFAULT_MAX_LEDS = 100;      ///< Default maximum number of LEDs

    hid_device_*    device;             ///< Handle to the HID device
    std::string     device_path;        ///< System path of the device
    std::string     device_name;        ///< Name of the device
    std::string     device_serial;      ///< Serial number of the device
    uint16_t        vid;                ///< Vendor ID
    uint16_t        pid;                ///< Product ID
    int             max_leds;           ///< Maximum number of LEDs

    /**
     * @brief Calculates the CRC8 checksum (MAXIM polynomial).
     * @param data Pointer to the data buffer.
     * @param size Size of the data.
     * @return The calculated CRC8 checksum.
     */
    uint8_t CalculateCRC8(const uint8_t* data, uint8_t size) const;

    /**
     * @brief Sends RGB data to the device.
     * @param rgb_data Pointer to the buffer of RGB data.
     * @param offset The starting offset of the LEDs.
     * @return true on success, false on failure.
     */
    bool SendRGBData(const uint8_t* rgb_data, int offset);

    /**
     * @brief Sends the end command to finalize the color update.
     * @param total_leds The total number of LEDs being updated.
     * @return true on success, false on failure.
     */
    bool SendEndCommand(int total_leds);
};
