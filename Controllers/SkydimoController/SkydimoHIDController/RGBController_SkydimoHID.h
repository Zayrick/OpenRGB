/*---------------------------------------------------------*\
| RGBController_SkydimoHID.h                                |
|                                                           |
|   RGBController for Skydimo HID LED Strip                 |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SkydimoHIDController.h"

/**
 * @brief OpenRGB控制器类，用于Skydimo HID LED灯带
 * @details 继承自RGBController，实现OpenRGB接口
 */
class RGBController_SkydimoHID : public RGBController
{
public:
    RGBController_SkydimoHID(SkydimoHIDController* controller_ptr);
    ~RGBController_SkydimoHID();

    /**
     * @brief 设置区域
     * @details 创建一个包含可变数量LED的灯带区域
     */
    void SetupZones();

    /**
     * @brief 调整区域大小
     * @param zone 区域索引
     * @param new_size 新的大小
     * @note 此设备支持调整LED数量
     */
    void ResizeZone(int zone, int new_size);

    /**
     * @brief 更新设备所有LED
     * @details 将所有LED颜色发送到设备
     */
    void DeviceUpdateLEDs();

    /**
     * @brief 更新指定区域的LED
     * @param zone 区域索引
     */
    void UpdateZoneLEDs(int zone);

    /**
     * @brief 更新单个LED
     * @param led LED索引
     * @note 此设备必须更新所有LED
     */
    void UpdateSingleLED(int led);

    /**
     * @brief 更新设备模式
     * @details 此设备仅支持直接控制模式
     */
    void DeviceUpdateMode();

private:
    SkydimoHIDController* controller;    ///< 硬件控制器指针
};
