# 3D HUD Engine - Mathematical Utilities

## Overview

The mathematical utilities library provides a comprehensive set of mathematical functions and algorithms organized into logical layers based on functionality and complexity. This modular architecture ensures optimal performance, maintainability, and ease of use for the 3D HUD engine.

## Directory Structure

```
inc/utils/math/
├── foundation/           # 基础数学工具层
│   ├── bit_ops.h        # 位运算工具 (is_power_of_two, align_up等)
│   ├── basic_math.h     # 基本数学函数 (clamp, lerp, 常量等)
│   └── constants.h      # 数学常量定义 (PI, EPSILON等)
├── geometry/            # 几何计算层
│   ├── vector_math.h    # 向量运算 (2D/3D/4D向量)
│   ├── matrix_math.h    # 矩阵运算 (变换、投影矩阵)
│   └── trigonometry.h   # 三角函数和几何计算
├── algorithms/          # 算法层
│   ├── curve_fitting/   # 曲线拟合算法
│   ├── interpolation/   # 插值算法
│   └── optimization/    # 优化算法
└── numeric/             # 数值计算层
    ├── integration/     # 数值积分方法
    └── differentiation/ # 数值微分方法
```

## Layer Design Philosophy

### Foundation Layer (基础工具层)
- **Purpose**: 提供高性能的基础数学运算
- **Characteristics**: O(1)复杂度，无依赖，极致性能
- **Usage**: 内存管理、数据结构、实时计算
- **Examples**: `is_power_of_two`, `align_up`, `clamp`, `lerp`

### Geometry Layer (几何计算层)
- **Purpose**: 3D图形和几何计算
- **Characteristics**: 向量化运算，SIMD优化
- **Usage**: 渲染管线、物理引擎、空间计算
- **Examples**: 向量变换、矩阵运算、几何相交检测

### Algorithms Layer (算法层)
- **Purpose**: 复杂数学算法实现
- **Characteristics**: 迭代算法，精度与性能权衡
- **Usage**: 动画系统、路径规划、数据分析
- **Examples**: 曲线拟合、插值算法、优化方法

### Numeric Layer (数值计算层)
- **Purpose**: 高级数值分析方法
- **Characteristics**: 高精度计算，数值稳定性
- **Usage**: 物理模拟、科学计算、工程应用
- **Examples**: 数值积分、微分方程求解

## Usage Examples

### Basic Usage
```cpp
#include "utils/math/math_utils.h"

using namespace hud_3d::utils::math;

// 基础数学运算
if (is_power_of_two(buffer_size)) {
    // 优化内存对齐
    size_t aligned_size = align_up(data_size, 16);
}

// 几何计算
float radians = degrees_to_radians(45.0f);
float clamped = clamp(value, 0.0f, 1.0f);
```

### Performance Considerations

#### Foundation Layer (极致性能)
- 所有函数标记为 `constexpr` 和 `noexcept`
- 使用模板实现编译时优化
- 零运行时开销，内联展开

#### Geometry Layer (SIMD优化)
- 利用硬件向量指令
- 批量数据处理
- 缓存友好的内存布局

#### Algorithms Layer (精度控制)
- 可配置的迭代次数
- 误差容忍度参数
- 性能与精度权衡选项

## Design Principles

### 1. **Single Responsibility Principle** (单一职责原则)
每个层专注于特定类型的数学运算，避免功能混杂。

### 2. **Performance First** (性能优先)
基础层函数设计为极致性能，适合高频调用场景。

### 3. **Type Safety** (类型安全)
使用模板确保类型安全，支持不同精度需求。

### 4. **Modular Dependencies** (模块化依赖)
高层模块可以依赖低层模块，但避免循环依赖。

## Implementation Status

### ✅ Completed
- **Foundation Layer**: Basic mathematical utilities and bit operations
- **File**: `math_utils.h` (main entry point)
- **File**: `foundation/bit_ops.h` (bit manipulation)
- **File**: `foundation/basic_math.h` (basic functions)

### 🔄 In Progress
- **Constants separation**: Moving constants to dedicated file
- **Geometry layer**: Vector and matrix mathematics
- **Integration**: Updating existing code to use new math library

### 📋 Planned
- **Geometry Layer**: Vector/matrix operations
- **Algorithms Layer**: Curve fitting and interpolation
- **Numeric Layer**: Numerical analysis methods
- **Testing**: Comprehensive unit test coverage

## Integration Guidelines

### For New Code
```cpp
// 推荐：使用完整的命名空间路径
#include "utils/math/math_utils.h"

void example_function() {
    using namespace hud_3d::utils::math;
    
    if (is_power_of_two(size)) {
        size_t aligned = align_up(size, alignment);
    }
}
```

### For Legacy Code Migration
```cpp
// 旧代码：类内数学函数
class MemoryPool {
    static bool isPowerOfTwo(uint64_t n);
};

// 新代码：使用数学工具库
class MemoryPool {
    // 移除数学函数，直接使用 math::is_power_of_two
};
```

## Best Practices

1. **Prefer Foundation Layer** for performance-critical code
2. **Use Template Specialization** for type-specific optimizations
3. **Enable Compiler Optimizations** (`-O2` or higher)
4. **Profile Performance** for algorithm selection
5. **Write Unit Tests** for mathematical correctness

## Contributing

When adding new mathematical functions:

1. **Categorize correctly** by functionality and complexity
2. **Follow naming conventions** (snake_case for functions)
3. **Add comprehensive documentation** with examples
4. **Include performance characteristics** in comments
5. **Provide unit tests** for validation

## License

This mathematical utilities library is part of the 3D HUD Engine project.
Copyright (c) 2024 3D HUD Project. All rights reserved.