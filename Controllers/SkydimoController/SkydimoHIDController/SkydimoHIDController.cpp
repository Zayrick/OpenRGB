/*---------------------------------------------------------*\
| SkydimoHIDController.cpp                                  |
|                                                           |
|   Driver for Skydimo HID LED Strip                        |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "SkydimoHIDController.h"
#include <hidapi.h>
#include <vector>
#include <array>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <sstream>

/**
 * @brief Constructor.
 * @details Initializes member variables.
 */
SkydimoHIDController::SkydimoHIDController()
{
    device          = nullptr;
    device_path     = "";
    device_name     = "Skydimo LED Strip";
    device_serial   = "000000";
    vid             = VID;
    pid             = PID;
    max_leds        = DEFAULT_MAX_LEDS;
}

/**
 * @brief Destructor.
 * @details Cleans up HID device resources.
 */
SkydimoHIDController::~SkydimoHIDController()
{
    CloseDevice();
}

/**
 * @brief Initializes the controller and opens the HID device.
 * @param path The system path of the HID device.
 * @return true on success, false on failure.
 */
bool SkydimoHIDController::Initialize(const std::string& path)
{
    device_path = path;

    device = hid_open_path(path.c_str());
    if (device == nullptr)
    {
        return false;
    }

    // Fetch device information
    wchar_t wstr[256];

    // Get product name
    if (hid_get_product_string(device, wstr, sizeof(wstr) / sizeof(wstr[0])) == 0)
    {
        // Convert wide string to multibyte string
        char product_name[256];
        wcstombs(product_name, wstr, sizeof(product_name));
        if (strlen(product_name) > 0)
        {
            device_name = "Skydimo " + std::string(product_name);
        }
    }

    // Get serial number
    if (hid_get_serial_number_string(device, wstr, sizeof(wstr) / sizeof(wstr[0])) == 0)
    {
        // Convert wide char serial to hex string
        std::ostringstream oss;
        oss << std::uppercase << std::hex << std::setfill('0');
        for (int i = 0; wstr[i] != 0 && i < 16; i++)
        {
            oss << std::setw(2) << static_cast<int>(wstr[i] & 0xFF);
        }

        std::string serial_hex = oss.str();
        if (!serial_hex.empty())
        {
            device_serial = serial_hex;
        }
    }

    return true;
}

/**
 * @brief Closes the HID device.
 */
void SkydimoHIDController::CloseDevice()
{
    if (device)
    {
        hid_close(device);
        device = nullptr;
    }
}

/**
 * @brief Sets the colors for the LEDs.
 * @param colors A vector of RGBColor structs.
 * @param count The number of LEDs to update.
 * @return true on success, false on failure.
 */
bool SkydimoHIDController::SetLEDs(const std::vector<RGBColor>& colors, int count)
{
    if (!device || colors.empty())
    {
        return false;
    }

    // Clamp LED count to a reasonable range
    int led_count = std::min(count, std::min(static_cast<int>(colors.size()), max_leds));

    // Send LED data in batches
    for (int idx = 0; idx < led_count; idx += BATCH_LEDS)
    {
        int current_batch_size = std::min(BATCH_LEDS, led_count - idx);

        // Prepare RGB data (in GRB order)
        std::array<uint8_t, MAX_RGB_BYTES> rgb_data_bytes{};
        for (int i = 0; i < current_batch_size; ++i)
        {
            RGBColor color = colors[idx + i];
            uint8_t r = RGBGetRValue(color);
            uint8_t g = RGBGetGValue(color);
            uint8_t b = RGBGetBValue(color);

            // Device uses GRB order
            rgb_data_bytes[i * 3]     = g;
            rgb_data_bytes[i * 3 + 1] = r;
            rgb_data_bytes[i * 3 + 2] = b;
        }

        // Send the current batch of RGB data
        if (!SendRGBData(rgb_data_bytes.data(), idx))
        {
            return false;
        }
    }

    // Send the end command
    return SendEndCommand(led_count);
}

/**
 * @brief Calculates the CRC8 checksum (MAXIM polynomial).
 * @param data Pointer to the data buffer.
 * @param size Size of the data.
 * @return The calculated CRC8 checksum.
 */
uint8_t SkydimoHIDController::CalculateCRC8(const uint8_t* data, uint8_t size) const
{
    constexpr uint8_t poly = 0x07;
    uint8_t crc = 0x00;

    for (uint8_t i = 0; i < size; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j)
        {
            if (crc & 0x80)
            {
                crc = static_cast<uint8_t>(((crc << 1) & 0xFF) ^ poly);
            }
            else
            {
                crc = static_cast<uint8_t>((crc << 1) & 0xFF);
            }
        }
    }

    return crc & 0xFF;
}

/**
 * @brief Sends RGB data to the device.
 * @param rgb_data Pointer to the buffer of RGB data.
 * @param offset The starting offset of the LEDs.
 * @return true on success, false on failure.
 */
bool SkydimoHIDController::SendRGBData(const uint8_t* rgb_data, int offset)
{
    std::vector<uint8_t> payload;
    payload.reserve(MAX_RGB_BYTES + 4); // Reserve enough space

    // Build the packet: command byte + offset (little-endian) + RGB data
    payload.push_back(0x01);
    payload.push_back(static_cast<uint8_t>(offset & 0xFF));
    payload.push_back(static_cast<uint8_t>((offset >> 8) & 0xFF));

    // Add RGB data
    payload.insert(payload.end(), rgb_data, rgb_data + MAX_RGB_BYTES);

    // Append CRC8 checksum
    payload.push_back(CalculateCRC8(payload.data(), static_cast<uint8_t>(payload.size())));

    // Send the packet
    int result = hid_write(device, payload.data(), static_cast<int>(payload.size()));
    return result >= 0;
}

/**
 * @brief Sends the end command to finalize the color update.
 * @param total_leds The total number of LEDs being updated.
 * @return true on success, false on failure.
 */
bool SkydimoHIDController::SendEndCommand(int total_leds)
{
    std::vector<uint8_t> end_payload;
    end_payload.reserve(MAX_RGB_BYTES + 1);

    // Build the end command: command byte + end marker + LED count (little-endian)
    end_payload.push_back(0x01);
    end_payload.push_back(0xFF);
    end_payload.push_back(0xFF);
    end_payload.push_back(static_cast<uint8_t>(total_leds & 0xFF));
    end_payload.push_back(static_cast<uint8_t>((total_leds >> 8) & 0xFF));

    // Pad to the required length
    end_payload.resize(MAX_RGB_BYTES, 0x00);

    // Append CRC8 checksum
    end_payload.push_back(CalculateCRC8(end_payload.data(), MAX_RGB_BYTES));

    // Send the end command
    int result = hid_write(device, end_payload.data(), static_cast<int>(end_payload.size()));
    return result >= 0;
}
