/*---------------------------------------------------------*\
| SkydimoHIDControllerDetect.cpp                            |
|                                                           |
|   Detector for Skydimo HID LED controllers                |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <vector>
#include <memory>
#include <hidapi.h>
#include "Detector.h"

// HID控制器相关头文件
#include "SkydimoHIDController.h"
#include "RGBController_SkydimoHID.h"

// HID设备标识符
#define SKYDIMO_HID_VID 0x1A86
#define SKYDIMO_HID_PID 0xE316

/******************************************************************************************\
*                                                                                          *
*   DetectSkydimoHIDControllers                                                            *
*                                                                                          *
*       检测Skydimo HID LED控制器设备                                                       *
*                                                                                          *
\******************************************************************************************/

void DetectSkydimoHIDControllers(hid_device_info* info, const std::string& /*name*/)
{
    try
    {
        // 使用智能指针保证异常安全
        std::unique_ptr<SkydimoHIDController> controller = std::make_unique<SkydimoHIDController>();

        // 尝试打开设备
        if (!controller->OpenDevice(info->path))
        {
            // 无法打开则跳过此设备
            return;
        }

        // 创建RGBController（OpenRGB管理生命周期，需释放所有权）
        RGBController_SkydimoHID* rgb_controller = new RGBController_SkydimoHID(controller.release());

        // 如果设备名称仍是默认值，添加路径信息便于识别
        if (rgb_controller->name == "Skydimo LED Strip")
        {
            rgb_controller->name += " (HID: " + std::string(info->path) + ")";
        }

        // 注册到OpenRGB资源管理器
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
    catch (const std::exception& e)
    {
        // 捕获异常，避免单个设备初始化失败影响其他设备
        // 可以在这里记录日志，但不中断检测过程
    }
}

/*---------------------------------------------------------------------------------------------------------*\
| 注册检测器                                                                                                 |
\*---------------------------------------------------------------------------------------------------------*/

// 注册HID设备检测器
REGISTER_HID_DETECTOR("Skydimo HID LED", DetectSkydimoHIDControllers, SKYDIMO_HID_VID, SKYDIMO_HID_PID);

/*---------------------------------------------------------------------------------------------------------*\
| UDEV规则条目（Linux）                                                                                      |
|                                                                                                           |
| DUMMY_DEVICE_DETECTOR("Skydimo HID LED", DetectSkydimoHIDControllers, 0x1A86, 0xE316 )                    |
\*---------------------------------------------------------------------------------------------------------*/
