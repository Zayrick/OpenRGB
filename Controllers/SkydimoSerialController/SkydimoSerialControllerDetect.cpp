/*---------------------------------------------------------*\
| SkydimoSerialControllerDetect.cpp                         |
|                                                           |
|   Detector for Skydimo Serial LED controllers             |
|                                                           |
|   Skydimo                                      2024-12-28 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <vector>
#include <memory>
#include "Detector.h"
#include "find_usb_serial_port.h"

// 串口控制器相关头文件
#include "SkydimoSerialController.h"
#include "RGBController_SkydimoSerial.h"

// 串口设备标识符
#define SKYDIMO_SERIAL_VID 0x1A86
#define SKYDIMO_SERIAL_PID 0x7523

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

            // 若设备型号未知，附加端口信息方便识别
            if (rgb_controller->name == "Skydimo LED Strip")
            {
                rgb_controller->name += " (Serial: " + *port + ")";
            }

            ResourceManager::get()->RegisterRGBController(rgb_controller);
        }
        catch (const std::exception& e)
        {
            // 捕获异常，避免单个设备初始化失败影响其他设备
        }
    }
}

/*---------------------------------------------------------------------------------------------------------*\
| 注册检测器                                                                                                 |
\*---------------------------------------------------------------------------------------------------------*/

// 注册串口设备检测器
REGISTER_DETECTOR("Skydimo Serial LED", DetectSkydimoSerialControllers);

/*---------------------------------------------------------------------------------------------------------*\
| UDEV规则条目（Linux）                                                                                      |
|                                                                                                           |
| DUMMY_DEVICE_DETECTOR("Skydimo Serial LED", DetectSkydimoSerialControllers, 0x1A86, 0x7523 )               |
\*---------------------------------------------------------------------------------------------------------*/
