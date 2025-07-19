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
 * @brief 构造函数
 * @details 初始化成员变量
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
 * @brief 析构函数
 * @details 清理HID设备资源
 */
SkydimoHIDController::~SkydimoHIDController()
{
    CloseDevice();
}

/**
 * @brief 打开HID设备
 * @param path HID设备路径
 * @return true 打开成功, false 打开失败
 */
bool SkydimoHIDController::OpenDevice(const std::string& path)
{
    device_path = path;

    device = hid_open_path(path.c_str());
    if (device == nullptr)
    {
        return false;
    }

    // 获取设备信息
    wchar_t wstr[256];

    // 获取产品名称
    if (hid_get_product_string(device, wstr, sizeof(wstr) / sizeof(wstr[0])) == 0)
    {
        // 将宽字符转换为多字节字符串
        char product_name[256];
        wcstombs(product_name, wstr, sizeof(product_name));
        if (strlen(product_name) > 0)
        {
            device_name = "Skydimo " + std::string(product_name);
        }
    }

    // 获取序列号
    if (hid_get_serial_number_string(device, wstr, sizeof(wstr) / sizeof(wstr[0])) == 0)
    {
        // 将宽字符序列号转换为十六进制字符串
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
 * @brief 关闭HID设备
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
 * @brief 设置LED颜色
 * @param colors RGB颜色数组
 * @param count LED数量
 * @return true 设置成功, false 设置失败
 */
bool SkydimoHIDController::SetLEDs(const std::vector<RGBColor>& colors, int count)
{
    if (!device || colors.empty())
    {
        return false;
    }

    // 限制LED数量在合理范围内
    int led_count = std::min(count, std::min(static_cast<int>(colors.size()), max_leds));

    // 分批发送LED数据
    for (int idx = 0; idx < led_count; idx += BATCH_LEDS)
    {
        int current_batch_size = std::min(BATCH_LEDS, led_count - idx);

        // 准备RGB数据（使用GRB顺序）
        std::array<uint8_t, MAX_RGB_BYTES> rgb_data_bytes{};
        for (int i = 0; i < current_batch_size; ++i)
        {
            RGBColor color = colors[idx + i];
            uint8_t r = RGBGetRValue(color);
            uint8_t g = RGBGetGValue(color);
            uint8_t b = RGBGetBValue(color);

            // 设备使用GRB顺序
            rgb_data_bytes[i * 3]     = g;
            rgb_data_bytes[i * 3 + 1] = r;
            rgb_data_bytes[i * 3 + 2] = b;
        }

        // 发送当前批次的RGB数据
        if (!SendRGBData(rgb_data_bytes.data(), idx))
        {
            return false;
        }
    }

    // 发送结束命令
    return SendEndCommand(led_count);
}

/**
 * @brief 计算CRC8校验值（MAXIM多项式）
 * @param data 数据缓冲区
 * @param size 数据大小
 * @return CRC8校验值
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
 * @brief 发送RGB数据到设备
 * @param rgb_data RGB数据缓冲区
 * @param offset LED偏移量
 * @return true 发送成功, false 发送失败
 */
bool SkydimoHIDController::SendRGBData(const uint8_t* rgb_data, int offset)
{
    std::vector<uint8_t> payload;
    payload.reserve(MAX_RGB_BYTES + 4); // 预留足够空间

    // 构建数据包：命令字节 + 偏移量（小端序）+ RGB数据
    payload.push_back(0x01);
    payload.push_back(static_cast<uint8_t>(offset & 0xFF));
    payload.push_back(static_cast<uint8_t>((offset >> 8) & 0xFF));

    // 添加RGB数据
    payload.insert(payload.end(), rgb_data, rgb_data + MAX_RGB_BYTES);

    // 添加CRC8校验
    payload.push_back(CalculateCRC8(payload.data(), static_cast<uint8_t>(payload.size())));

    // 发送数据包
    int result = hid_write(device, payload.data(), static_cast<int>(payload.size()));
    return result >= 0;
}

/**
 * @brief 发送结束命令
 * @param total_leds 总LED数量
 * @return true 发送成功, false 发送失败
 */
bool SkydimoHIDController::SendEndCommand(int total_leds)
{
    std::vector<uint8_t> end_payload;
    end_payload.reserve(MAX_RGB_BYTES + 1);

    // 构建结束命令：命令字节 + 结束标记 + LED数量（小端序）
    end_payload.push_back(0x01);
    end_payload.push_back(0xFF);
    end_payload.push_back(0xFF);
    end_payload.push_back(static_cast<uint8_t>(total_leds & 0xFF));
    end_payload.push_back(static_cast<uint8_t>((total_leds >> 8) & 0xFF));

    // 填充到指定长度
    end_payload.resize(MAX_RGB_BYTES, 0x00);

    // 添加CRC8校验
    end_payload.push_back(CalculateCRC8(end_payload.data(), MAX_RGB_BYTES));

    // 发送结束命令
    int result = hid_write(device, end_payload.data(), static_cast<int>(end_payload.size()));
    return result >= 0;
}
