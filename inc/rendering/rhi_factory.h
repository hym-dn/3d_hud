#pragma once

#include "rhi_device.h"
#include <memory>

namespace RHI {

// 命名空间别名
using platform = hud_3d::platform;

// 图形API类型
enum class GraphicsAPI {
    OpenGL,
    OpenGL_ES,  // 注意：下划线，与platform::ContextAPI保持一致
    Vulkan,
    Direct3D,
    Metal
};

// 设备创建配置
struct DeviceConfig {
    GraphicsAPI api = GraphicsAPI::OpenGL;
    uint32_t major_version = 0;
    uint32_t minor_version = 0;
    bool debug_mode = false;
    bool vsync = true;
    
    // 平台特定配置
    platform::IGraphicsContext* platform_context = nullptr; // 图形上下文
    void* platform_surface = nullptr; // 平台表面句柄，例如：EGLSurface, WGL HDC等
};

// RHI工厂类
class RHIFactory {
public:
    // 创建RHI设备
    static Result<std::unique_ptr<IRHIDevice>> CreateDevice(const DeviceConfig& config);
    
    // 检测支持的图形API
    static std::vector<GraphicsAPI> GetSupportedAPIs();
    
    // 获取默认配置
    static DeviceConfig GetDefaultConfig();
    
    // 检测平台
    static const char* GetPlatformName();
    
    // 检测图形API版本
    static Result<std::pair<uint32_t, uint32_t>> DetectAPIInfo(GraphicsAPI api);
};

} // namespace RHI