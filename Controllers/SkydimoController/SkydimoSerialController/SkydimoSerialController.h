/*---------------------------------------------------------*\
| | SkydimoSerialController.h                                 |
| |                                                           |
| |   Driver for the Skydimo Serial LED Strip.                |
| |                                                           |
| |   Skydimo                                      2024-12-28 |
| |                                                           |
| |   This file is part of the OpenRGB project.               |
| |   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include "serial_port.h"
#include "RGBController.h"
#include <thread>
#include <atomic>
#include <mutex>

/**
 * @brief Controller class for the Skydimo Serial LED strip.
 * @details This class handles communication with the LED strip over a serial port,
 *          supporting up to 100 LEDs.
 */
class SkydimoSerialController
{
public:
    SkydimoSerialController();
    ~SkydimoSerialController();

    /**
     * @brief Initializes the controller and opens the serial port.
     * @param portname The name of the serial port (e.g., "COM3" or "/dev/ttyUSB0").
     * @return true on success, false on failure.
     */
    bool Initialize(const std::string& portname);

    /**
     * @brief Gets the device name.
     * @return A string containing the device name.
     */
    std::string GetDeviceName();

    /**
     * @brief Gets the device serial number.
     * @return A string containing the serial number.
     */
    std::string GetSerial();

    /**
     * @brief Gets the device location (serial port path).
     * @return A string containing the port name.
     */
    std::string GetLocation();

    /**
     * @brief Sets the colors for the LEDs.
     * @param colors A vector of RGBColor structs.
     */
    void SetLEDs(const std::vector<RGBColor>& colors);

    /**
     * @brief Starts a keep-alive thread to periodically resend the last colors.
     */
    void StartKeepAlive();

    /**
     * @brief Stops the keep-alive thread.
     */
    void StopKeepAlive();

    /**
     * @brief Gets the number of LEDs supported by the device.
     * @return The number of LEDs.
     */
    int GetLEDCount() { return num_leds; }

private:
    serial_port*    serialport;         ///< Pointer to the serial port object
    std::string     port_name;          ///< Name of the serial port
    std::string     device_name;        ///< Name of the device
    std::string     device_serial;      ///< Serial number of the device
    int             num_leds;           ///< Number of LEDs

    /*-----------------------------------------------------*\
    | Members for the keep-alive mechanism                  |
    \*-----------------------------------------------------*/
    std::thread              keep_alive_thread;   ///< Keep-alive thread
    std::atomic<bool>        keep_alive_running;  ///< Flag to control the thread loop
    std::mutex               write_mutex;         ///< Mutex for serial port writes
    std::vector<RGBColor>    last_colors;         ///< The last set of colors sent

    /**
     * @brief Low-level function to send color data. Does not modify last_colors
     *        and assumes the caller holds the lock.
     */
    void SendColors(const std::vector<RGBColor>& colors);

    /**
     * @brief The main loop for the keep-alive thread.
     */
    void KeepAliveLoop();

    /**
     * @brief Retrieves device information (name, serial, etc.).
     * @return true on success, false on failure.
     */
    bool GetDeviceInfo();
};
