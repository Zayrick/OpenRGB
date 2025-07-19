/*---------------------------------------------------------*\
| SkydimoSerialController.h                                 |
|                                                           |
|   Driver for Skydimo Serial LED Strip                     |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
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
 * @brief Skydimo串口LED控制器类
 * @details 通过串口协议控制LED灯带，支持100个灯珠
 */
class SkydimoSerialController
{
public:
    SkydimoSerialController();
    ~SkydimoSerialController();

    /**
     * @brief 初始化串口设备
     * @param portname 串口端口名称
     * @return true 初始化成功, false 初始化失败
     */
    bool Initialize(const std::string& portname);

    /**
     * @brief 获取设备名称
     * @return 设备名称字符串
     */
    std::string GetDeviceName();

    /**
     * @brief 获取设备序列号
     * @return 设备序列号字符串
     */
    std::string GetSerial();

    /**
     * @brief 获取设备位置（串口路径）
     * @return 设备位置字符串
     */
    std::string GetLocation();

    /**
     * @brief 设置LED颜色
     * @param colors RGB颜色数组
     */
    void SetLEDs(const std::vector<RGBColor>& colors);

    /**
     * @brief 启动保活线程，在后台周期性重发上次颜色
     */
    void StartKeepAlive();

    /**
     * @brief 停止保活线程
     */
    void StopKeepAlive();

    /**
     * @brief 获取LED数量
     * @return LED数量
     */
    int GetLEDCount() { return num_leds; }

private:
    serial_port*    serialport;         ///< 串口对象指针
    std::string     port_name;          ///< 串口名称
    std::string     device_name;        ///< 设备名称
    std::string     device_serial;      ///< 设备序列号
    int             num_leds;           ///< LED数量

    /*-----------------------------------------------------*\
    | 保活机制相关成员                                      |
    \*-----------------------------------------------------*/
    std::thread              keep_alive_thread;   ///< 保活线程
    std::atomic<bool>        keep_alive_running;  ///< 线程运行标志
    std::mutex               write_mutex;         ///< 串口写互斥
    std::vector<RGBColor>    last_colors;         ///< 最近一次发送的颜色

    /**
     * @brief 底层发送颜色数据，不修改 last_colors，调用方需要保证加锁
     */
    void SendColors(const std::vector<RGBColor>& colors);

    /**
     * @brief 保活线程循环函数
     */
    void KeepAliveLoop();

    /**
     * @brief 获取设备信息
     * @return true 获取成功, false 获取失败
     */
    bool GetDeviceInfo();
};
