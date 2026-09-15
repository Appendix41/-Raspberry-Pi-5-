# Raspberry Pi 5 + ESP32 九轴 IMU 入门与运行手册

本项目用于在树莓派 5 上通过 Arduino IDE 给微雪电子 `General Driver for Robots` 开发板中的 ESP32 编译和烧录程序，并读取板载九轴 IMU 数据。

本手册是在以下资料和实际调试结果的基础上整理的：

- [原始 FlowUs 教程：基于树莓派的 ESP32 开发](https://flowus.cn/share/37c63cfd-e1af-4244-9766-cb81d897ac7e)
- [微雪电子：General Driver for Robots 官方资料](https://www.waveshare.net/wiki/General_Driver_for_Robots)
- [Espressif：Arduino-ESP32 官方安装说明](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [Arduino 官方软件下载页](https://www.arduino.cc/en/software)


## 🌟 特别鸣谢

<table>
  <tr>
    <td width="180" align="center">
      <a href="https://api.entropicecho.com/">
        <img src="https://doc.entropicecho.com/img/entropicecho-icon.png" width="110" alt="熵之回响">
      </a>
    </td>
    <td>
      <a href="https://api.entropicecho.com/"><strong>熵之回响</strong></a><br><br>
      感谢 <strong>熵之回响</strong> 对本项目开发过程提供的支持。
      在资料整理、代码分析、问题排查与开发调试过程中，熵之回响提供的API服务为项目推进提供了便利与帮助。
    </td>
  </tr>
</table>

<br>

## 1. 先理解整个系统

这套系统里有三层设备，各自负责不同的工作：

| 设备 | 作用 | 可以怎样理解 |
| --- | --- | --- |
| 树莓派 5 | 运行 Arduino IDE、编译代码、上传程序、显示串口数据 | 开发电脑 |
| ESP32 | 执行烧录进去的程序，通过 I2C 读取 IMU | 真正运行嵌入式程序的控制器 |
| 板载 IMU | 测量加速度、转动速度和磁场方向 | ESP32 的感觉器官 |

USB 线同时承担两项工作：

1. 给开发板供电；
2. 在树莓派和 ESP32 之间传输程序及串口数据。

这里的“九轴”不是九个独立传感器，而是九路测量量：

- QMI8658：三轴加速度 + 三轴陀螺仪，共六轴；
- AK09918：三轴磁力计，共三轴；
- 合计九轴，所以示例名为 `9DOF_Demo`。

## 2. 使用的硬件

| 硬件 | 数量 | 用途 |
| --- | ---: | --- |
| 树莓派 5 | 1 | 安装 Arduino IDE、编译、烧录和查看串口 |
| 树莓派电源、存储卡和显示输入设备 | 1 套 | 运行 Raspberry Pi OS |
| 微雪 `General Driver for Robots` 开发板 | 1 | 板上包含 ESP32 和九轴 IMU |
| 支持数据传输的 USB 线 | 1 | 连接树莓派与 ESP32；只有充电功能的线不能烧录 |

本示例使用开发板板载 IMU，一般不需要另外接四根杜邦线。代码内部使用：

| 信号 | ESP32 引脚 | 用途 |
| --- | --- | --- |
| SDA | GPIO32 | I2C 数据线 |
| SCL | GPIO33 | I2C 时钟线 |

如果你使用的不是这块开发板，而是外接独立 IMU，必须同时连接 `3.3V`、`GND`、`SDA/GPIO32`、`SCL/GPIO33`。不要把只支持 3.3V 的传感器直接接到 5V。

## 3. 项目目录说明

本次真正要运行的是：


目录可以这样理解：

```text
1.IMU/
├─ README.md
├─ .gitignore
├─ 代码/
│  └─ 9DOF_Demo/
│     ├─ 9DOF_Demo.ino       Arduino 主程序入口
│     ├─ IMU.h / IMU.cpp     初始化、读取、校准和姿态融合
│     ├─ QMI8658.h / .cpp    加速度计和陀螺仪驱动
│     ├─ QMI8658reg.h         QMI8658 寄存器定义
│     └─ AK09918.h / .cpp     磁力计驱动
└─ ardiuno配置/              本地保存的安装资料，目录名沿用原文件
   ├─ packages/               已整理的离线 ESP32 开发包
   ├─ esp32安装/              安装包及另一份展开文件
   └─ 依赖库/                 其他实验可能使用的 Arduino 库
```


### 源码文件的作用

| 文件 | 作用 |
| --- | --- |
| `9DOF_Demo.ino` | 调用 IMU 初始化函数，然后每约 100 ms 读取并打印一次数据 |
| `IMU.cpp` | 配置 I2C、执行磁力计校准、调用姿态融合算法、计算 Roll/Pitch/Yaw |
| `IMU.h` | 声明角度和三轴数据结构，以及对外使用的函数 |
| `QMI8658.cpp/.h` | 通过 I2C 访问 QMI8658，读取加速度和角速度 |
| `QMI8658reg.h` | 保存芯片寄存器编号和配置项 |
| `AK09918.cpp/.h` | 通过 I2C 访问 AK09918，读取三轴磁场数据 |

目录中的 `.vscode`、`*.svd`、`debug.cfg` 和 `debug_custom.json` 是旧电脑和调试器使用的配置。普通 Arduino IDE 编译、烧录和串口查看都不需要它们。

## 4. 第一次使用：安装 Arduino IDE

如果树莓派上已经能打开 Arduino IDE 1.8.19，可以跳到第 5 节。

### 第 1 步：打开终端

在 Raspberry Pi OS 桌面顶部点击终端图标，或者从应用菜单打开 `Terminal`。

用途：后面的安装命令需要由 Linux 终端执行。

### 第 2 步：更新软件索引

```bash
sudo apt update
```

用途：让树莓派重新获取软件列表，避免因为本地列表过旧而找不到 Arduino。

### 第 3 步：安装 Arduino IDE

<img width="778" height="529" alt="image" src="https://github.com/user-attachments/assets/de4a73c0-50a9-44e0-8903-db2c01ccbbd7" />


```bash
sudo apt install arduino
```

看到是否继续的提示时输入 `Y`，再按回车。

用途：安装用于编辑、编译、烧录和查看串口的 Arduino IDE。原实验实际使用的是 Arduino IDE `1.8.19`。

### 第 4 步：确认安装

在树莓派终端输入 arduino
应看到弹出 arduino界面
<img width="309" height="284" alt="image" src="https://github.com/user-attachments/assets/c43a37cb-b5f6-49d9-b951-8ddfd060c785" />


## 5. 第一次使用：安装 ESP32 开发板支持（如果用现有代码，则已配置好，可跳过）

只安装 Arduino IDE 还不够。Arduino IDE 默认不知道怎样为 ESP32 编译程序，因此还要安装 ESP32 开发板包。

### 方法 A：使用官方开发板管理器

这是公开项目推荐的方法，因为不需要下载本仓库中几 GB 的离线工具链。

1. 打开 Arduino IDE。
2. 点击 `文件 -> 首选项`。
3. 在“附加开发板管理器网址”中加入：

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

4. 点击 `确定`。
5. 点击 `工具 -> 开发板 -> 开发板管理器`。
6. 搜索 `esp32`。
7. 选择 `esp32 by Espressif Systems`，为复现实验优先选择 `2.0.11`，然后点击 `安装`。
8. 安装完成后关闭并重新打开 Arduino IDE。

用途：开发板包提供 ESP32 编译器、烧录工具、芯片定义和基础库。没有它，菜单里不会出现 `ESP32 Dev Module`。

### 方法 B：使用原教程整理的离线 packages

只有在官方开发板管理器下载失败，并且你持有本地完整资料时才使用此方法。

1. 完全关闭 Arduino IDE。
2. 打开树莓派文件管理器。
3. 按 `Ctrl+H` 显示隐藏文件。
4. 进入自己的主目录，例如 `/home/appendix41`。
5. 找到隐藏目录 `.arduino15`；没有就新建。
6. 先备份 `.arduino15` 中已有的 `packages`，避免覆盖其他开发板环境。
7. 从本地项目的 `ardiuno配置/packages` 复制整个 `packages` 文件夹到 `.arduino15`。
8. 重新打开 Arduino IDE，查看 `工具 -> 开发板` 中是否出现 `ESP32 Arduino`。

用途：这相当于手动安装一套已经为 Linux 整理过的 ESP32 编译和烧录工具。

注意：不能把 Windows 开发包中的 `.exe` 文件简单复制到树莓派运行。Windows 和 Linux 使用不同格式的可执行文件；开发包还必须与树莓派的 CPU 架构匹配。优先使用方法 A，让开发板管理器自动选择正确版本。

## 6. 依赖库安装
在树莓派5中的文件管理器中找到该路径：

`/home/username/Arduino`
下载文件`libraries`，并进行解压缩，

将`libraries`文件粘贴到该路径下，如图所示，检查依赖库是否被`arduino`识别，若`libraries`中的文件均在加载库中显示，则证明依赖库安装成功

<img width="1836" height="890" alt="image" src="https://github.com/user-attachments/assets/4a9f3cfb-29be-4d85-ad10-c029b0d038c6" />


## 7. 连接开发板并确认串口


### 第 1 步：连接 USB

用支持数据传输的 USB 线连接树莓派和 `General Driver for Robots` 开发板。（一定是能传数据的，不是充电线）
<img width="934" height="1046" alt="image" src="https://github.com/user-attachments/assets/39e31f12-e498-401b-b212-da1c6aebfed0" />
接线如图，注意输入电压为12v

用途：建立供电、程序上传和串口输出通道。

### 第 2 步：在终端检查设备

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

正常情况下会看到类似：

```text
/dev/ttyUSB0
```

用途：确认 Linux 已经识别开发板。`ttyUSB0` 只是常见名称，实际也可能是 `ttyUSB1` 或 `ttyACM0`，必须以自己的树莓派输出为准。

### 第 3 步：处理串口权限

如果 Arduino IDE 提示无权打开串口，执行：

```bash
sudo usermod -aG dialout "$USER"
```

然后注销并重新登录，或者重启树莓派。

用途：把当前用户加入允许访问串口的 `dialout` 用户组。只执行命令但不重新登录，权限通常不会立即生效。

## 8. 打开并配置 9DOF_Demo

在arduino界面，选择开发板ESP32 Dev Module，作为我们的开发环境
<img width="1055" height="909" alt="image" src="https://github.com/user-attachments/assets/5d790b98-591d-4c8a-a239-feb2a2f02fd3" />

用途：这些参数决定生成怎样的固件以及怎样写入 ESP32。对本示例来说，开发板类型、Flash 大小和端口最关键。

### 第 4 步：选择端口

点击 `工具 -> 端口`，选择第 7 节实际查到的端口，例如 `/dev/ttyUSB0`。
<img width="773" height="724" alt="image" src="https://github.com/user-attachments/assets/7c1f04d0-2aba-47b5-b3fa-195ce048a3d9" />


用途：告诉 Arduino IDE 应该向哪一个 USB 串口上传程序。

不要照抄别人电脑上的 `COM115`。`COM115` 是原 `.vscode/arduino.json` 中保存的 Windows 端口，在树莓派上无效。

## 9. 编译、烧录和运行

### 第 1 步：编译验证

点击 Arduino IDE 左上角的对勾按钮“验证”。

用途：把 C++ 源代码翻译成 ESP32 能执行的机器程序，并检查语法、库和开发板环境是否完整。验证只在树莓派上生成程序，不会改变 ESP32 中原有的程序。

看到“编译完成”后再继续。如果这里报错，优先解决编译环境，不要反复按上传。

### 第 2 步：上传，也就是烧录

点击右箭头按钮“上传”。

Arduino IDE 会先重新编译，再通过 USB 把生成的固件写进 ESP32 的 Flash 存储器。这个写入过程就是“烧录”。

成功时通常能看到：

```text
Writing at ...
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
```

如果一直停在 `Connecting...`：

1. 确认选中了正确端口；
2. 关闭占用端口的串口监视器或其他程序；
3. 把 Upload Speed 改为 `115200`；
4. 必要时按住开发板 `BOOT`，开始上传后再松开。

### 第 3 步：打开串口监视器

点击：

```text
工具 -> 串口监视器
```

右下角波特率选择：

```text
115200
```

用途：查看 ESP32 正在运行的程序输出。波特率不一致会显示乱码。

### 第 4 步：先放平，再按 RESET

1. 把开发板平稳放在桌面上；
2. 远离磁铁、扬声器、大电机、铁制工具和强电流导线；
3. 串口监视器保持打开；
4. 按一下开发板上的 `EN` 或 `RESET`。

用途：程序一启动就会做加速度计、陀螺仪和磁力计校准。先打开串口再复位，才能看到完整提示并及时完成动作。

## 10. 按正确时间完成启动校准（待开发，目前会直接跳过这一过程，即无校准，直接采集）

这一段最容易操作错误。原程序不会等你按确认键，每个姿态只固定等待约 4 秒。

### 阶段 1：保持静止和平放

启动时可能看到：

```text
Position your ICM20948 flat and don't move it - calibrating...
```

虽然文字写的是 `ICM20948`，实际代码读取的是 QMI8658。这是原驱动中没有改干净的旧提示，不代表传感器型号错误。

操作：保持开发板平放且完全不动。

用途：程序采集静止数据，用于估计加速度计和陀螺仪的零偏。此时移动会导致静止时仍出现明显角速度。

### 阶段 2：磁力计第一次采样

看到：

```text
keep 10dof-imu device horizontal and it will read x y z axis offset value after 4 seconds
```

操作：继续正面朝上、水平放置约 4 秒。

用途：记录第一个方向的磁场值。

### 阶段 3：绕 Z 轴旋转 180 度

看到：

```text
rotate z axis 180 degrees and it will read all axises offset value after 4 seconds
```

操作：在桌面平面内把开发板转半圈，保持正面朝上，再保持不动。

用途：采集相反水平方向的磁场值，用于估算 X、Y 轴偏置。这里是像转动桌面上的书一样旋转，不是把板子翻面。

### 阶段 4：翻面并保持水平

看到：

```text
flip 10dof-imu device and keep it horizontal and it will read all axises offset value after 4 seconds
```

操作：把开发板上下翻面，然后水平放置并保持不动。

用途：采集相反竖直方向的磁场值，用于估算 Z 轴偏置。

如果动作来不及、板子碰动或方向做错，直接保持串口监视器打开，再按一次 `RESET`，从头重新校准即可。

## 11. 看懂串口输出

校准结束后会持续输出：

```text
Roll : ...    Pitch : ...    Yaw : ...
Acceleration: X : ...    Acceleration: Y : ...    Acceleration: Z : ...
Gyroscope: X : ...       Gyroscope: Y : ...       Gyroscope: Z : ...
Magnetic: X : ...        Magnetic: Y : ...        Magnetic: Z : ...
```

| 输出 | 含义 | 当前程序中的单位 |
| --- | --- | --- |
| Roll | 左右侧倾角 | 度 |
| Pitch | 前后俯仰角 | 度 |
| Yaw | 水平朝向角 | 度 |
| Acceleration X/Y/Z | 三轴加速度 | mg，约 1000 mg 等于 1 g |
| Gyroscope X/Y/Z | 三轴角速度 | dps，即度/秒 |
| Magnetic X/Y/Z | 三轴磁场 | 近似微特斯拉，驱动中进行了整数换算 |

驱动还会单独打印 `mg` 和 `dps`。这是 `QMI8658.cpp` 中遗留的调试输出，不是报错。

### 最小验证动作

1. 静止放置 10 秒：陀螺仪三轴应在 0 附近小幅波动，角度不应快速连续发散。
2. 只向前后倾斜：Pitch 应明显变化。
3. 只向左右倾斜：Roll 应明显变化。
4. 保持水平并在桌面上转动：Yaw 和磁力计数据应变化。
5. 恢复原姿态：Roll/Pitch 应大致回到原值，允许存在小幅误差。

这些检查只能证明模块基本可用，不能证明达到医疗测量精度。

## 12. 常见问题排查

| 现象 | 正在解决的问题 | 操作 |
| --- | --- | --- |
| 菜单里没有 ESP32 | ESP32 开发板包未安装 | 回到第 5 节安装，重启 Arduino IDE |
| 没有 `/dev/ttyUSB0` | USB 线、接口或设备识别失败 | 换一根确认能传数据的线，换 USB 口，再运行端口检查命令 |
| `Permission denied` | 当前 Linux 用户没有串口权限 | 加入 `dialout` 组并重新登录 |
| 上传停在 `Connecting...` | ESP32 未进入下载模式或串口不稳 | 改 115200、检查端口，必要时配合 `BOOT` 按键 |
| 串口乱码 | 串口监视器波特率错误 | 设置为 115200 |
| 串口没有开机提示 | 打开监视器时程序已经启动完 | 保持监视器打开并按 `RESET` |
| `qmi8658_init fail` | ESP32 没读到 QMI8658 | 检查板卡和供电；代码地址固定为 `0x6B`，其他地址会失败 |
| `AK09918_init fail` | ESP32 没读到磁力计 | 检查硬件；程序之后可能一直显示 `Waiting Sensor` |
| Roll/Pitch/Yaw 为 `nan` | 姿态融合拿到了无效数据 | 重新复位校准，先确认三类原始数据都在变化 |
| Yaw 跳变或偏差很大 | 磁场干扰或校准动作错误 | 远离磁性物体和电机，重新按第 10 节校准 |
| 静止角度持续漂移 | 零偏、采样时间或算法误差 | 重新静止校准；参考第 14 节的算法限制 |

### Arduino 1.8.19 编译器出现 panic

如果出现：

```text
panic: runtime error: index out of range [3] with length 3
```

并同时看到 `ESP Insights`、`ESP RainMaker`、`WiFiProv` 的分类警告，可在树莓派终端执行：

```bash
while IFS= read -r fp; do
  if grep -q '^category=' "$fp"; then
    sed -i 's/^category=.*/category=Other/' "$fp"
  else
    sed -i '/^architectures=/i category=Other' "$fp"
  fi
  echo "fixed: $fp"
done < <(grep -rl -E '^name=(ESP Insights|ESP RainMaker|WiFiProv)$' \
  "$HOME/.arduino15/packages/esp32" --include=library.properties 2>/dev/null)
```

然后清理临时构建缓存：

```bash
rm -rf /tmp/arduino/sketches/*
rm -rf "$HOME/.arduino15/cache/"*
```

完全关闭并重新打开 Arduino IDE，再点击“验证”。

用途：给缺失或无效的库分类补上 Arduino 1.8.19 能识别的值，并删除旧的解析缓存。单独删除 `includes.cache` 不一定能解决开发板包元数据本身的问题。

## 13. 可能遇到的问题：可以编译，但不能上传

出现这个问题的原因是当前版本的Arduino 1.8.19的缓存不能重复读取，最简单的办法是完全退出之后，不编译直接上传。
