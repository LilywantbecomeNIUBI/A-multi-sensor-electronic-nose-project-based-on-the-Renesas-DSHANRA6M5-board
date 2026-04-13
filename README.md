# A-multi-sensor-electronic-nose-project-based-on-the-Renesas-DSHANRA6M5-board
基于瑞萨 DSHANRA6M5 开发板的多传感器电子鼻项目，使用 Edge Impulse 训练模型并完成 RA6M5 端侧部署，支持数据采集、气味识别与开源部署教程。

# Multi-Sensor Electronic Nose on Renesas DSHANRA6M5

基于瑞萨 **DSHANRA6M5 / RA6M5** 开发板的多传感器电子鼻项目。  
本项目完成了从**多传感器采集**、**特征窗口构建**、**Edge Impulse 模型部署**到**端侧实时识别与 LVGL 表情交互显示**的完整闭环。

> 这是一个面向嵌入式边缘 AI 的开源实践项目，包含：
> - 传感器驱动与采样链路
> - 业务状态机与数据处理
> - Edge Impulse 模型在 RA6M5 上的部署
> - 基于 LVGL 的表情显示界面
> - 后续将持续补充部署教程与 ChatGPT 辅助开发经验

---

## 1. Project Overview

传统单一气体传感器难以稳定识别复杂气味，而电子鼻的思路是通过**多传感器交叉敏感响应 + 模式识别**完成分类。  
本项目基于 RA6M5 平台，构建了一个多传感器信息融合的端侧气味识别系统，能够对不同气味进行本地推理与实时反馈。

当前系统主要支持以下识别类别：

- `air`
- `alcohol`
- `perfume`
- `vinegar`

这些类别也在推理代码中以固定标签形式定义。

---

## 2. Features

- 基于 **Renesas RA6M5 / DSHANRA6M5** 的嵌入式实现
- 多传感器融合采集
- 支持 **MQ 系列气体传感器 + SHT40 + SGP40**
- 基于 **Edge Impulse** 训练并部署模型到 MCU 端
- 端侧本地推理，无需云端依赖
- 使用 **LVGL** 实现识别结果的表情化显示
- 具备较清晰的模块化软件架构，便于二次开发

---

## 3. Hardware Architecture

本项目的硬件系统由传感层、处理层和输出层组成。根据项目设计材料，系统使用多路 MQ 传感器阵列作为气味响应主体，并结合环境补偿与 VOC 辅助信息，在 RA6M5 上完成本地推理。

### Main Components

- **Renesas RA6M5 / DSHANRA6M5**
- **ADS1115**：用于多路模拟气体传感器采样
- **MQ2 / MQ3 / MQ135 / MQ138**
- **SHT40**：温湿度补偿
- **SGP40**：VOC 指数辅助输入
- **LCD**：显示识别结果与表情
- **雾化/喷雾模块**：配合采样流程形成标准化激励过程

在代码中，`sensor_hub` 模块统一组织了 MQ 采样链、状态机、SHT40/SGP40 联合服务与记录结构。

---

## 4. Software Architecture

项目整体采用了较明显的模块化设计，核心可以理解为四层：

### 4.1 Driver Layer
底层驱动层负责各类硬件外设与传感器访问，例如：

- `ads1115.*`
- `sht40.*`
- `sgp40.*`
- `sensirion_*`
- `st7789.*`
- `i2c_master.*`
- `drv_gpt_timer.*`

### 4.2 Sensor / Middleware Layer
用于整合采样、状态管理与环境补偿，例如：

- `mq_acquire.*`
- `mq_metrics.*`
- `mq_state_machine.*`
- `sht40_sgp40_service.*`
- `sensor_hub.*`

其中 `sensor_hub_record_t` 统一封装了单轮采样记录，包含：

- MQ 原始值与电压值
- baseline / delta / ratio / peak 等中间量
- 温湿度
- SGP40 原始值
- VOC Index
- I2C 状态与错误信息

这些内容都在 `sensor_hub.h` 中有明确结构定义。

### 4.3 Inference Layer
推理相关代码主要包含：

- `ei_infer.*`
- `app_infer.*`

其中：
- `ei_infer.cpp` 负责将采样窗口转换为 Edge Impulse 所需输入，并调用 `run_classifier()` 执行推理。
- `app_infer.c` 负责业务层决策映射，将模型输出转换为 `air / alcohol / perfume / vinegar / uncertain` 等语义结果。

### 4.4 UI Layer
显示部分基于 **LVGL**，核心文件为：

- `face_simple.c`

该模块实现了多种表情状态与动画，包括：

- Happy
- Sad
- Alert
- Thinking
- Air

并将识别结果映射为表情和底部状态文本，例如：

- `Perfume -> Happy`
- `Vinegar -> Sad`
- `Alcohol -> Alert`
- `Detecting -> Thinking`
- `Air -> Air`

相关逻辑可见 `face_simple_set_emotion()`。

---

## 5. Sensor Pipeline

本项目的采样与处理流程如下：

1. 采集 MQ 阵列的模拟输出
2. 通过 ADS1115 转换为数字量
3. 采集 SHT40 温湿度数据
4. 采集 SGP40 VOC 相关数据
5. 在 `sensor_hub` 中形成统一记录
6. 由 `app_infer` 提取模型输入
7. 使用 Edge Impulse 模型执行分类
8. 通过串口/LCD/LVGL 表情反馈结果

`sensor_hub.h` 中明确说明了当前统一采样接口 `SensorHub_SampleOnce()` 用于完成一次完整采样；同时假设外层以 **1 秒周期**调用，以满足 VOC 算法的 **1Hz** 采样要求。

---

## 6. Edge Impulse Deployment

本项目的一个重点是：**将通过 Edge Impulse 训练得到的模型成功部署到 Renesas RA6M5 平台**。这也是本仓库后续会重点补充教程的内容。

### 6.1 Input Design

在 `ei_infer.cpp` 中，推理输入由以下 5 路特征按时间顺序拼接：

- `env_voc_index`
- `mq135_mv`
- `mq138_mv`
- `mq2_mv`
- `mq3_mv`

代码中会把环形窗口展开成一帧完整输入后，再交给 Edge Impulse 的 `run_classifier()`。

### 6.2 Window-Based Inference

推理采用滑动窗口策略：

- 窗口未满时：持续收集数据
- 窗口满后：执行一次完整推理

代码中通过 `EIInfer_IsWindowReady()` 判断窗口是否准备完成，并在 `EIInfer_Run()` 中正式调用模型。

### 6.3 Output Labels

当前模型输出标签为：

- `air`
- `alcohol`
- `perfume`
- `vinegar`

这些标签直接定义在 `ei_infer.cpp` 中。

### 6.4 Decision Layer

`app_infer.c` 在模型输出基础上增加了业务决策层：

- Top-1 置信度不足时，返回 `uncertain`
- 否则将类别索引映射到具体气味类别

这使得最终输出更适合设备端业务使用。

---

## 7. LVGL Expression UI

为了让识别结果更直观，本项目没有只输出字符串，而是使用 LVGL 构建了一个“电子鼻表情脸”。

UI 设计特点包括：

- 左右眼、眉毛、嘴型的组合变化
- 支持眨眼动画
- 支持 thinking 状态的问号云朵动画
- 底部状态文本实时显示识别类别
- 不同气味对应不同情绪反馈

`face_simple.c` 中实现了多种表情绘制函数，如：

- `face_simple_apply_happy()`
- `face_simple_apply_sad()`
- `face_simple_apply_alert()`
- `face_simple_apply_thinking()`
- `face_simple_apply_air()`

并使用 LVGL 对象、定时器和标签来构建动态交互。

---

## 8. Repository Structure

下面是一个建议理解方式，具体以仓库实际目录为准：

```text
.
├── src/                    # 核心业务代码、传感器服务、推理与 UI
│   ├── sensor_hub.*        # 统一传感器采样与记录
│   ├── mq_*                # MQ 采样、特征与状态机
│   ├── sht40_* / sgp40_*   # 环境补偿与 VOC 服务
│   ├── ei_infer.*          # Edge Impulse 推理封装
│   ├── app_infer.*         # 推理结果业务决策
│   ├── face_simple.*       # LVGL 表情 UI
│   └── st7789.*            # LCD 显示驱动
├── ra/                     # RA 工程相关文件
├── ra_cfg/                 # FSP 配置
├── ra_gen/                 # 自动生成代码
├── lvgl/                   # LVGL 图形库
├── script/                 # 脚本或辅助工具
├── README.md
└── LICENSE
