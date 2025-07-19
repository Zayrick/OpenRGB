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
 * @brief 构造函数
 * @details 初始化成员变量
 */
SkydimoSerialController::SkydimoSerialController()
{
    serialport      = nullptr;
    port_name       = "";
    device_name     = "Skydimo LED Strip";
    device_serial   = "000000";
    num_leds        = 100;  // 默认100个灯珠
    keep_alive_running = false;
}

/**
 * @brief 析构函数
 * @details 清理串口资源
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
 * @brief 初始化串口设备
 * @param portname 串口端口名称
 * @return true 初始化成功, false 初始化失败
 */
bool SkydimoSerialController::Initialize(const std::string& portname)
{
    port_name = portname;

    /*-----------------------------------------------------*\
    | 仅打开一次串口，避免重复打开导致端口被占用                     |
    \*-----------------------------------------------------*/
    serialport = new serial_port();

    // 设置并打开串口，115200-8-N-1，无流控
    if(!serialport->serial_open(port_name.c_str(), 115200))
    {
        delete serialport;
        serialport = nullptr;
        return false;
    }

    // 成功打开后尝试读取设备信息（如失败也不影响后续使用）
    GetDeviceInfo();

    // 不再自动启动保活线程，由上层模式控制决定
    return true;
}

/**
 * @brief 获取设备名称
 * @return 设备名称字符串
 */
std::string SkydimoSerialController::GetDeviceName()
{
    return device_name;
}

/**
 * @brief 获取设备序列号
 * @return 设备序列号字符串
 */
std::string SkydimoSerialController::GetSerial()
{
    return device_serial;
}

/**
 * @brief 获取设备位置
 * @return 串口路径
 */
std::string SkydimoSerialController::GetLocation()
{
    return port_name;
}

/**
 * @brief 设置LED颜色
 * @param colors RGB颜色数组
 * @details 使用与原代码相同的协议格式
 */
void SkydimoSerialController::SetLEDs(const std::vector<RGBColor>& colors)
{
    if (!serialport || colors.empty())
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lk(write_mutex);
        last_colors = colors; // 保存最后一次颜色
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
    const auto interval = 250ms; // 250ms 周期保活

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
 * @brief 获取设备信息
 * @return true 获取成功, false 获取失败
 * @details 发送Moni-A命令获取设备型号和序列号
 */
bool SkydimoSerialController::GetDeviceInfo()
{
    if(!serialport)
    {
        return false;
    }

    // 发送查询命令
    const char* cmd = "Moni-A";
    // 将 size_t 显式转换为 int，避免 VS 在 64 位环境下产生 C4267 警告
    int cmd_len = static_cast<int>(strlen(cmd));
    serialport->serial_write((char*)cmd, cmd_len);

    // 等待响应
    #ifdef _WIN32
    Sleep(10);
    #else
    usleep(10000);
    #endif

    // 读取响应
    char buf[64] = {0};
    // sizeof 返回 size_t，需要显式转换为 int
    int bytes_read = serialport->serial_read(buf, static_cast<int>(sizeof(buf)));

    if(bytes_read > 0)
    {
        // 解析响应格式: "型号,序列号\r\n"
        std::string response(buf, bytes_read);
        size_t comma_pos = response.find(',');

        if(comma_pos != std::string::npos)
        {
            // 提取型号
            std::string model = response.substr(0, comma_pos);
            if(!model.empty())
            {
                device_name = "Skydimo " + model;
            }

            // 提取序列号
            size_t end_pos = response.find_first_of("\r\n", comma_pos);
            if(end_pos == std::string::npos)
            {
                end_pos = response.length();
            }

            if(end_pos > comma_pos + 1)
            {
                // 将原始字节序列转换为十六进制字符串，确保非 ASCII 字符不会导致乱码
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
