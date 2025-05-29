# OpenGL 学习项目

这是一个包含多个 OpenGL 学习示例和练习的项目，主要使用 C++ 和现代 OpenGL 技术。

## 项目结构

```
├── README.md                   # 项目说明文件
├── test_glm.cpp               # GLM 数学库测试
├── test_plus.cpp              # 基础测试程序
├── .vscode/                   # Visual Studio Code 配置
│   ├── c_cpp_properties.json  # C++ 智能感知配置
│   ├── launch.json            # 调试配置
│   ├── settings.json          # 编辑器设置
│   └── tasks.json             # 构建任务配置
├── book_exercise/             # 教材练习代码
├── cube_array_test/           # 立方体数组测试
├── glm/                       # GLM 数学库
├── learning_opengl/           # OpenGL 学习代码
├── mouse_with_circle/         # 鼠标交互圆形示例
├── old_program/               # 早期程序代码
└── test_for_shader/           # 着色器测试代码
```

## 主要功能模块

### 📚 教材练习 ([book_exercise/](book_exercise/))
- **2_1.cpp**: 基础 OpenGL 窗口创建和初始化
- **2_2.cpp**: 硬编码着色器程序创建
- **2_3.cpp**: 顶点着色器和片段着色器编译
- **2_4.cpp**: 从文件读取着色器源码
- **test.cpp**: 交互式彩色三角形绘制
  - 支持鼠标左键改变颜色
  - 支持鼠标右键缩放和旋转

### 🎯 OpenGL 学习 ([learning_opengl/](learning_opengl/))
- **1.cpp**: 粒子系统实现
  - 2000个粒子的实时渲染
  - 重力物理模拟
  - 透明度和尺寸动画
  - 圆形粒子渲染
- **3.cpp**: 基础图形渲染
- **1_.cpp**: 窗口管理和上下文设置

### 🧪 测试代码
- **test_glm.cpp**: GLM 数学库功能测试
- **test_plus.cpp**: 基础功能测试
- **test_for_shader/1.cpp**: GLchar 类型测试

### 🎮 交互示例
- **mouse_with_circle/**: 鼠标与圆形的交互示例
- **cube_array_test/**: 立方体数组渲染测试

## 技术栈

- **图形API**: OpenGL 3.3+ / 4.1+
- **窗口管理**: GLFW
- **OpenGL加载**: GLEW
- **数学库**: GLM (OpenGL Mathematics)
- **编程语言**: C++
- **开发环境**: Visual Studio Code

## 主要特性

### 🎨 着色器系统
- 顶点着色器和片段着色器编译
- 从文件读取着色器源码
- 着色器程序链接和错误处理

### 🎯 交互功能
- 鼠标事件处理
- 实时颜色变化
- 动态缩放和旋转

### ⚡ 粒子系统
- 大规模粒子渲染 (最多2000个粒子)
- 物理模拟 (重力、速度)
- 生命周期管理
- 透明度渐变效果

### 📐 数学计算
- 三角函数应用
- 矩阵变换
- 向量运算

## 编译说明

确保你的系统已安装以下依赖：

### Windows
```bash
# 安装必要的库
# GLFW, GLEW, GLM

# 编译示例 (使用 g++)
g++ -o program.exe source.cpp -lglfw3 -lglew32 -lopengl32
```

### 配置要求
- OpenGL 3.3+ 支持
- C++11 或更高版本
- GLFW 3.x
- GLEW
- GLM

## 使用说明

### 基础程序运行
```bash
# 编译并运行基础示例
g++ -o test.exe book_exercise/test.cpp -lglfw3 -lglew32 -lopengl32
./test.exe
```

### 粒子系统演示
```bash
# 编译粒子系统
g++ -o particles.exe learning_opengl/1.cpp -lglfw3 -lglew32 -lopengl32
./particles.exe
```

## 交互控制

### 三角形示例 (test.cpp)
- **鼠标左键**: 随机改变三角形颜色
- **鼠标右键**: 放大三角形并旋转
- **ESC键**: 退出程序

### 粒子系统 (1.cpp)
- 自动生成和更新粒子
- 重力效果模拟
- 实时透明度变化

## 学习要点

1. **OpenGL 基础**
   - 上下文创建和管理
   - 顶点缓冲对象 (VBO)
   - 顶点数组对象 (VAO)

2. **着色器编程**
   - GLSL 语法
   - 顶点和片段着色器
   - Uniform 变量使用

3. **数学应用**
   - 三角函数绘制圆形
   - 矩阵变换
   - 向量运算

4. **交互设计**
   - 鼠标事件处理
   - 实时更新渲染

## 常见问题

### 编译错误
- 确保已正确安装 GLFW、GLEW 和 GLM
- 检查 OpenGL 驱动程序是否支持所需版本

### 运行时错误
- 验证着色器文件路径是否正确
- 检查 OpenGL 上下文是否成功创建

## 贡献指南

欢迎提交问题报告和改进建议！

## 许可证

本项目仅用于学习目的。

---

**开发环境**: Visual Studio Code  
**最后更新**: 2025年