/*---------------------------------------------------------*\
| SkydimoControllerDetect.cpp                              |
|                                                           |
|   Detector for Skydimo LED controllers                    |
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
#include "find_usb_serial_port.h"
#include "LogManager.h"

// HID控制器相关头文件
#include "SkydimoHIDController/SkydimoHIDController.h"
#include "SkydimoHIDController/RGBController_SkydimoHID.h"

// 串口控制器相关头文件
#include "SkydimoSerialController/SkydimoSerialController.h"
#include "SkydimoSerialController/RGBController_SkydimoSerial.h"

// HID设备标识符
#define SKYDIMO_HID_VID 0x1A86
#define SKYDIMO_HID_PID 0xE316

// 串口设备标识符
#define SKYDIMO_SERIAL_VID 0x1A86
#define SKYDIMO_SERIAL_PID 0x7523

// 设备名称常量
constexpr auto DEFAULT_DEVICE_NAME = "Skydimo LED Strip";

/******************************************************************************************\
*                                                                                          *
*   RegisterControllerWithIdentifier                                                       *
*                                                                                          *
*       通用设备注册函数，处理默认名称和标识符添加                                            *
*                                                                                          *
\******************************************************************************************/

template<typename RGBControllerType>
void RegisterControllerWithIdentifier(RGBControllerType* rgb_controller, const std::string& identifier)
{
    // 如果设备名称仍是默认值，添加标识信息便于识别
    if (rgb_controller->name == DEFAULT_DEVICE_NAME)
    {
        rgb_controller->name += " " + identifier;
    }

    // 注册到OpenRGB资源管理器
    ResourceManager::get()->RegisterRGBController(rgb_controller);
}


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

        // 使用辅助函数注册控制器
        RegisterControllerWithIdentifier(rgb_controller, "(HID: " + std::string(info->path) + ")");
    }
    catch (const std::exception& e)
    {
        // 记录异常信息，避免单个设备初始化失败影响其他设备
        LOG_ERROR("Failed to initialize Skydimo HID controller at path %s: %s", info->path, e.what());
    }
}

/******************************************************************************************\
*                                                                                          *
*   DetectSkydimoSerialControllers                                                         *
*                                                                                          *
*       检测Skydimo串口LED控制器设备                                                        *
*                                                                                          *
\******************************************************************************************/

void DetectSkydimoSerialControllers()
{
    // 查找所有匹配 VID/PID 的串口设备
    std::vector<std::string*> ports = find_usb_serial_port(SKYDIMO_SERIAL_VID, SKYDIMO_SERIAL_PID);

    for (auto* port_ptr : ports)
    {
        // 提前释放端口字符串，防止内存泄漏
        std::unique_ptr<std::string> port(port_ptr);

        if (!port || port->empty())
        {
            continue;
        }

        try
        {
            // 使用智能指针保证异常安全
            std::unique_ptr<SkydimoSerialController> controller = std::make_unique<SkydimoSerialController>();

            if (!controller->Initialize(*port))
            {
                // 无法初始化则跳过
                continue;
            }

            // RGBController 由 OpenRGB 生命周期管理，需释放所有权
            RGBController_SkydimoSerial* rgb_controller = new RGBController_SkydimoSerial(controller.release());

            // 使用辅助函数注册控制器
            RegisterControllerWithIdentifier(rgb_controller, "(Serial: " + *port + ")");
        }
        catch (const std::exception& e)
        {
            // 记录异常信息，避免单个设备初始化失败影响其他设备
            LOG_ERROR("Failed to initialize Skydimo Serial controller on port %s: %s", port->c_str(), e.what());
        }
    }
}

/*---------------------------------------------------------------------------------------------------------*\
| 注册检测器                                                                                                 |
\*---------------------------------------------------------------------------------------------------------*/

// 注册HID设备检测器
REGISTER_HID_DETECTOR("Skydimo HID LED", DetectSkydimoHIDControllers, SKYDIMO_HID_VID, SKYDIMO_HID_PID);

// 注册串口设备检测器
REGISTER_DETECTOR("Skydimo Serial LED", DetectSkydimoSerialControllers);

/*---------------------------------------------------------------------------------------------------------*\
| UDEV规则条目（Linux）                                                                                      |
|                                                                                                           |
| DUMMY_DEVICE_DETECTOR("Skydimo HID LED", DetectSkydimoHIDControllers, 0x1A86, 0xE316 )                    |
| DUMMY_DEVICE_DETECTOR("Skydimo Serial LED", DetectSkydimoSerialControllers, 0x1A86, 0x7523 )               |
\*---------------------------------------------------------------------------------------------------------*/
