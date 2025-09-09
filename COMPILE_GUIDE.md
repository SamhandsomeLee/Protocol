# ERNC Protocol 编译指南

## 编译步骤

### 1. 环境准备
```bash
# Windows (MSYS2) - 设置环境变量绕过Qt许可证检查
export QTFRAMEWORK_BYPASS_LICENSE_CHECK=1
```

### 2. 执行编译
```bash
# 进入protocol目录
cd protocol

# 创建构建目录
mkdir build-gcc && cd build-gcc

# 配置CMake
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# 执行编译
cmake --build . --parallel 4
```

### 3. 验证结果
编译成功后会生成：
- `libprotocol.dll` - 动态库 (~3.6MB)
- `libprotocol_static.a` - 静态库 (~7.7MB)
- `examples/` 目录下的示例程序

## 遇到的问题及解决方案

### 问题1: Qt许可证错误
**错误信息:**
```
Qt requires a commercial or open source license to continue.
```

**解决方案:**
```bash
export QTFRAMEWORK_BYPASS_LICENSE_CHECK=1
```

### 问题2: AutoMoc警告
**警告信息:**
```
Warning: .cpp file contains "moc_" include
```

**说明:** 这是Qt的AutoMoc系统产生的警告，不影响编译结果，可以忽略。

### 问题3: 头文件找不到
**错误信息:**
```
fatal error: mapping/parameter_mapper.h: No such file or directory
```

**解决方案:**
问题出现在CMakeLists.txt配置中，已通过修正包含目录配置解决：
```cmake
target_include_directories(ProtocolLibStatic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/nanopb
    ${CMAKE_CURRENT_SOURCE_DIR}/messages
)
```

### 问题4: Qt模块找不到
**错误信息:**
```
Could not find a package configuration file provided by "Qt6SerialPort"
```

**解决方案:**
确保安装了Qt6的SerialPort模块：
```bash
# MSYS2
pacman -S mingw-w64-x86_64-qt6-serialport

# 或设置Qt路径
export Qt6_DIR=/path/to/qt6/lib/cmake/Qt6
```

## 系统要求

- **操作系统**: Windows 10/11
- **编译器**: GCC 15.1.0+ (MSYS2/MinGW-w64)
- **CMake**: 3.20+
- **Qt**: 6.2+ (Core, SerialPort模块)

## 构建输出

成功编译后的目录结构：
```
build-gcc/
├── libprotocol.dll              # 动态库
├── libprotocol_static.a         # 静态库
├── examples/
│   ├── basic_example.exe        # 基础示例
│   ├── advanced_example.exe     # 高级示例
│   ├── di_example.exe           # 依赖注入示例
│   └── ring_buffer_example.exe  # 环形缓冲区示例
```

## 测试编译结果

```bash
# 运行基础示例程序
./examples/basic_example.exe

# 检查动态库依赖
ldd libprotocol.dll  # Linux
# 或
dumpbin /dependents libprotocol.dll  # Windows
```

---

**注意**: 本指南基于实际编译过程中遇到的问题总结而成，适用于Windows MSYS2环境。
