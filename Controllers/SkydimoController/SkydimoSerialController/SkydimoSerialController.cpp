/*---------------------------------------------------------*\
| SkydimoSerialController.cpp                               |
|                                                           |
|   Driver for Skydimo Serial LED Strip                     |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "SkydimoSerialController.h"
#include "RGBController.h"
#include <cstring>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

/**
 * @brief Constructor.
 * @details Initializes member variables.
 */
SkydimoSerialController::SkydimoSerialController()
{
    serialport      = nullptr;
    port_name       = "";
    device_name     = "Skydimo LED Strip";
    device_serial   = "000000";
    num_leds        = 100;  // Default to 100 LEDs
    keep_alive_running = false;
}

/**
 * @brief Destructor.
 * @details Cleans up serial port resources.
 */
SkydimoSerialController::~SkydimoSerialController()
{
    StopKeepAlive();
    if(serialport)
    {
        delete serialport;
        serialport = nullptr;
    }
}

/**
 * @brief Initializes the serial device.
 * @param portname The name of the serial port.
 * @return true on success, false on failure.
 */
bool SkydimoSerialController::Initialize(const std::string& portname)
{
    port_name = portname;

    // Open the serial port only once to avoid conflicts.
    serialport = new serial_port();

    // Set up and open the serial port: 115200-8-N-1, no flow control.
    if(!serialport->serial_open(port_name.c_str(), 115200))
    {
        delete serialport;
        serialport = nullptr;
        return false;
    }

    // Try to retrieve device info after successful open.
    // Failure to do so does not prevent subsequent operations.
    GetDeviceInfo();

    // The keep-alive thread is no longer started automatically;
    // it's controlled by the selected mode in the RGB controller.
    return true;
}

/**
 * @brief Gets the device name.
 * @return A string containing the device name.
 */
std::string SkydimoSerialController::GetDeviceName()
{
    return device_name;
}

/**
 * @brief Gets the device serial number.
 * @return A string containing the serial number.
 */
std::string SkydimoSerialController::GetSerial()
{
    return device_serial;
}

/**
 * @brief Gets the device location (serial port).
 * @return A string containing the serial port path.
 */
std::string SkydimoSerialController::GetLocation()
{
    return port_name;
}

/**
 * @brief Sets the colors of the LEDs.
 * @param colors A vector of RGBColor structs.
 * @details Uses the same protocol format as the original implementation.
 */
void SkydimoSerialController::SetLEDs(const std::vector<RGBColor>& colors)
{
    if (!serialport || colors.empty())
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lk(write_mutex);
        last_colors = colors; // Save the last set colors
        SendColors(colors);
    }
}

void SkydimoSerialController::SendColors(const std::vector<RGBColor>& colors)
{
    if (!serialport)
    {
        return;
    }

    int count = std::min(static_cast<int>(colors.size()), num_leds);

    std::vector<unsigned char> packet;
    packet.reserve(6 + count * 3);

    packet.insert(packet.end(), {
        0x41, 0x64, 0x61, 0x00,
        static_cast<unsigned char>((count >> 8) & 0xFF),
        static_cast<unsigned char>(count & 0xFF)
    });

    for (int i = 0; i < count; ++i)
    {
        packet.push_back(RGBGetRValue(colors[i]));
        packet.push_back(RGBGetGValue(colors[i]));
        packet.push_back(RGBGetBValue(colors[i]));
    }

    serialport->serial_write(reinterpret_cast<char*>(packet.data()), static_cast<int>(packet.size()));
}

void SkydimoSerialController::StartKeepAlive()
{
    if (keep_alive_running)
    {
        return;
    }

    keep_alive_running = true;
    keep_alive_thread = std::thread(&SkydimoSerialController::KeepAliveLoop, this);
}

void SkydimoSerialController::StopKeepAlive()
{
    if (!keep_alive_running)
    {
        return;
    }

    keep_alive_running = false;
    if (keep_alive_thread.joinable())
    {
        keep_alive_thread.join();
    }
}

void SkydimoSerialController::KeepAliveLoop()
{
    using namespace std::chrono_literals;
    const auto interval = 250ms; // 250ms keep-alive interval

    while (keep_alive_running)
    {
        {
            std::lock_guard<std::mutex> lk(write_mutex);
            if (!last_colors.empty())
            {
                SendColors(last_colors);
            }
        }
        std::this_thread::sleep_for(interval);
    }
}

/**
 * @brief Retrieves device information.
 * @return true on success, false on failure.
 * @details Sends the "Moni-A" command to get the device model and serial.
 */
bool SkydimoSerialController::GetDeviceInfo()
{
    if(!serialport)
    {
        return false;
    }

    // Send query command
    const char* cmd = "Moni-A";
    // Explicitly cast size_t to int to avoid C4267 warning in VS on 64-bit
    int cmd_len = static_cast<int>(strlen(cmd));
    serialport->serial_write((char*)cmd, cmd_len);

    // Wait for response
    #ifdef _WIN32
    Sleep(10);
    #else
    usleep(10000);
    #endif

    // Read response
    char buf[64] = {0};
    // sizeof returns size_t, needs explicit cast to int
    int bytes_read = serialport->serial_read(buf, static_cast<int>(sizeof(buf)));

    if(bytes_read > 0)
    {
        // Parse response format: "Model,Serial\r\n"
        std::string response(buf, bytes_read);
        size_t comma_pos = response.find(',');

        if(comma_pos != std::string::npos)
        {
            // Extract model
            std::string model = response.substr(0, comma_pos);
            if(!model.empty())
            {
                device_name = "Skydimo " + model;
            }

            // Extract serial number
            size_t end_pos = response.find_first_of("\r\n", comma_pos);
            if(end_pos == std::string::npos)
            {
                end_pos = response.length();
            }

            if(end_pos > comma_pos + 1)
            {
                // Convert raw byte sequence to hex string to prevent issues
                // with non-ASCII characters.
                std::string serial_raw = response.substr(comma_pos + 1, end_pos - comma_pos - 1);
                std::ostringstream oss;
                oss << std::uppercase << std::hex << std::setfill('0');
                for(unsigned char ch : serial_raw)
                {
                    oss << std::setw(2) << static_cast<int>(ch);
                }
                device_serial = oss.str();
            }

            return true;
        }
    }

    return false;
}
