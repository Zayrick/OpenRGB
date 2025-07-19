/*---------------------------------------------------------*\
| RGBController_SkydimoSerial.cpp                           |
|                                                           |
|   RGBController for Skydimo Serial LED Strip              |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_SkydimoSerial.h"

/**
 * @brief 构造函数
 * @param controller_ptr 硬件控制器指针
 * @details 初始化控制器信息和模式
 */
RGBController_SkydimoSerial::RGBController_SkydimoSerial(SkydimoSerialController* controller_ptr)
{
    controller          = controller_ptr;

    // 设置设备信息
    name                = controller->GetDeviceName();
    vendor              = "Skydimo";
    type                = DEVICE_TYPE_LEDSTRIP;
    description         = "Skydimo Serial LED Strip Controller";
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

    // 创建关闭模式
    mode Off;
    Off.name            = "Off";
    Off.value           = 1;
    Off.flags           = 0;
    Off.color_mode      = MODE_COLORS_NONE;
    modes.push_back(Off);


    // 创建流式模式（带保活机制）
    mode Stream;
    Stream.name         = "Stream";
    Stream.value        = 2;
    Stream.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Stream.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Stream);
    SetupZones();
}

/**
 * @brief 析构函数
 * @details 清理硬件控制器
 */
RGBController_SkydimoSerial::~RGBController_SkydimoSerial()
{
    delete controller;
}

/**
 * @brief 设置区域
 * @details 创建一个包含100个LED的灯带区域
 */
void RGBController_SkydimoSerial::SetupZones()
{
    // 创建LED灯带区域
    zone strip_zone;
    strip_zone.name         = "LED Strip";
    strip_zone.type         = ZONE_TYPE_LINEAR;
    strip_zone.leds_min     = controller->GetLEDCount();
    strip_zone.leds_max     = controller->GetLEDCount();
    strip_zone.leds_count   = controller->GetLEDCount();
    strip_zone.matrix_map   = nullptr;
    zones.push_back(strip_zone);

    // 为每个LED创建LED对象
    for(int i = 0; i < controller->GetLEDCount(); i++)
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
 * @note 此设备不支持调整区域大小，保持100个LED固定
 */
void RGBController_SkydimoSerial::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | 此设备不支持调整区域大小                                    |
    | LED数量固定为100个                                          |
    \*---------------------------------------------------------*/
}

/**
 * @brief 更新设备所有LED
 * @details 将颜色数组发送到硬件
 */
void RGBController_SkydimoSerial::DeviceUpdateLEDs()
{
    controller->SetLEDs(colors);
}

/**
 * @brief 更新指定区域的LED
 * @param zone 区域索引
 * @details 由于只有一个区域，直接更新所有LED
 */
void RGBController_SkydimoSerial::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

/**
 * @brief 更新单个LED
 * @param led LED索引
 * @note 此设备必须一次更新所有LED，因此调用DeviceUpdateLEDs
 */
void RGBController_SkydimoSerial::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

/**
 * @brief 更新设备模式
 * @details 根据模式选择启动或停止保活机制，Off模式关闭所有LED
 */
void RGBController_SkydimoSerial::DeviceUpdateMode()
{
    // active_mode 是基类 RGBController 的成员，记录当前激活模式的 value
    if (active_mode == 1) // Off 模式
    {
        controller->StopKeepAlive();
        // 创建全黑颜色数组关闭所有LED
        std::vector<RGBColor> black_colors(controller->GetLEDCount(), ToRGBColor(0, 0, 0));
        controller->SetLEDs(black_colors);
    }
    else if (active_mode == 2) // Stream 模式
    {
        controller->StartKeepAlive();
    }
    else // Direct 模式 (active_mode == 0)
    {
        controller->StopKeepAlive();
    }
}
