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
 * @brief 构造函数
 * @param controller_ptr 硬件控制器指针
 * @details 初始化控制器信息和模式
 */
RGBController_SkydimoHID::RGBController_SkydimoHID(SkydimoHIDController* controller_ptr)
{
    controller          = controller_ptr;

    // 设置设备信息
    name                = controller->GetDeviceName();
    vendor              = "Skydimo";
    type                = DEVICE_TYPE_LEDSTRIP;
    description         = "Skydimo HID LED Strip Controller";
    version             = "1.0";
    serial              = controller->GetSerial();
    location            = controller->GetLocation();

    // 创建直接控制模式
    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = 0;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

/**
 * @brief 析构函数
 * @details 清理硬件控制器
 */
RGBController_SkydimoHID::~RGBController_SkydimoHID()
{
    delete controller;
}

/**
 * @brief 设置区域
 * @details 创建一个包含可变数量LED的灯带区域
 */
void RGBController_SkydimoHID::SetupZones()
{
    // 清除现有区域和LED
    zones.clear();
    leds.clear();

    // 创建LED灯带区域
    zone strip_zone;
    strip_zone.name         = "LED Strip";
    strip_zone.type         = ZONE_TYPE_LINEAR;
    strip_zone.leds_min     = 1;                               // 最小1个LED
    strip_zone.leds_max     = controller->GetMaxLEDCount();    // 最大支持LED数量
    strip_zone.leds_count   = controller->GetMaxLEDCount();    // 默认使用最大LED数量
    strip_zone.matrix_map   = nullptr;
    zones.push_back(strip_zone);

    // 为每个LED创建LED对象
    for(int i = 0; i < strip_zone.leds_count; i++)
    {
        led new_led;
        new_led.name = "LED " + std::to_string(i + 1);
        leds.push_back(new_led);
    }

    SetupColors();
}

/**
 * @brief 调整区域大小
 * @param zone 区域索引
 * @param new_size 新的大小
 * @details 支持动态调整LED数量
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

    // 重新设置LED列表
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
 * @brief 更新设备所有LED
 * @details 将颜色数组发送到硬件
 */
void RGBController_SkydimoHID::DeviceUpdateLEDs()
{
    if(!zones.empty())
    {
        controller->SetLEDs(colors, zones[0].leds_count);
    }
}

/**
 * @brief 更新指定区域的LED
 * @param zone 区域索引
 * @details 由于只有一个区域，直接更新所有LED
 */
void RGBController_SkydimoHID::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

/**
 * @brief 更新单个LED
 * @param led LED索引
 * @note 此设备必须一次更新所有LED，因此调用DeviceUpdateLEDs
 */
void RGBController_SkydimoHID::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

/**
 * @brief 更新设备模式
 * @details 此设备仅支持直接控制模式，无需特殊处理
 */
void RGBController_SkydimoHID::DeviceUpdateMode()
{
    // 仅支持直接控制模式，无需更新
}
