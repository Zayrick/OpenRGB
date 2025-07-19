/*---------------------------------------------------------*\
| SkydimoHIDController.h                                    |
|                                                           |
|   Driver for Skydimo HID LED Strip                        |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include "RGBController.h"

struct hid_device_;

/**
 * @brief Skydimo HID LED控制器类
 * @details 通过HID协议控制LED灯带，支持可变数量LED
 */
class SkydimoHIDController
{
public:
    SkydimoHIDController();
    ~SkydimoHIDController();

    /**
     * @brief 打开HID设备
     * @param path HID设备路径
     * @return true 打开成功, false 打开失败
     */
    bool OpenDevice(const std::string& path);

    /**
     * @brief 关闭HID设备
     */
    void CloseDevice();

    /**
     * @brief 设置LED颜色
     * @param colors RGB颜色数组
     * @param count LED数量
     * @return true 设置成功, false 设置失败
     */
    bool SetLEDs(const std::vector<RGBColor>& colors, int count);

    /**
     * @brief 获取设备名称
     * @return 设备名称字符串
     */
    std::string GetDeviceName() const { return device_name; }

    /**
     * @brief 获取设备序列号
     * @return 设备序列号字符串
     */
    std::string GetSerial() const { return device_serial; }

    /**
     * @brief 获取设备路径
     * @return 设备路径字符串
     */
    std::string GetLocation() const { return device_path; }

    /**
     * @brief 获取VID
     * @return 厂商ID
     */
    uint16_t GetVendorID() const { return vid; }

    /**
     * @brief 获取PID
     * @return 产品ID
     */
    uint16_t GetProductID() const { return pid; }

    /**
     * @brief 获取支持的最大LED数量
     * @return 最大LED数量
     */
    int GetMaxLEDCount() const { return max_leds; }

private:
    static constexpr uint16_t VID = 0x1A86;           ///< 厂商ID
    static constexpr uint16_t PID = 0xE316;           ///< 产品ID
    static constexpr int MAX_RGB_BYTES = 60;          ///< 每次写入的最大RGB字节数
    static constexpr int BATCH_LEDS = 20;             ///< 每批处理的LED数量
    static constexpr int DEFAULT_MAX_LEDS = 100;      ///< 默认最大LED数量

    hid_device_*    device;             ///< HID设备句柄
    std::string     device_path;        ///< 设备路径
    std::string     device_name;        ///< 设备名称
    std::string     device_serial;      ///< 设备序列号
    uint16_t        vid;                ///< 厂商ID
    uint16_t        pid;                ///< 产品ID
    int             max_leds;           ///< 最大LED数量

    /**
     * @brief 计算CRC8校验值（MAXIM多项式）
     * @param data 数据缓冲区
     * @param size 数据大小
     * @return CRC8校验值
     */
    uint8_t CalculateCRC8(const uint8_t* data, uint8_t size) const;

    /**
     * @brief 发送RGB数据到设备
     * @param rgb_data RGB数据缓冲区
     * @param offset LED偏移量
     * @return true 发送成功, false 发送失败
     */
    bool SendRGBData(const uint8_t* rgb_data, int offset);

    /**
     * @brief 发送结束命令
     * @param total_leds 总LED数量
     * @return true 发送成功, false 发送失败
     */
    bool SendEndCommand(int total_leds);
};
