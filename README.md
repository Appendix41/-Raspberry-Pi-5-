# -Raspberry-Pi-5-
选择树莓派5为上位机 ESP32 arduino uno为下位机，监测多种传感器数据
首先明确，我们所买的开发板为`微雪电子的[General Driver for Robots](https://www.waveshare.net/wiki/General_Driver_for_Robots)`，其官方提供教程只用基于`Windows`的版本，而我们选用的`树莓派5`的开发环境为`Linux`，由于官方不提供Linux下的开发教程(最生气的是不提供任何跟开发板有关的手册😒)，重新配置环境较为复杂，且部分文件下载速度较慢，接下来的教程所使用到的文件都是我整理好的，大家直接按照教程做就行❤️

# Arduino IDE 安装                                                     



由于`[General Driver for Robots](https://www.waveshare.net/wiki/General_Driver_for_Robots)`开发板的芯片为`esp32`，主流开发软件为`ESP-IDF`和`Ardiuno IDE`，这里我们选用`Arduino`来对`esp32`进行开发

打开树莓派5的终端，输入命令

`sudo apt-get install arduino`

输入Y然后回车，系统将自动安装`Arduino`,这里我安装的版本为`1.8.19`

![image.png](https://tc-cdn.flowus.cn/oss/948be399-8e68-41ad-9ef5-522f3938b1ee/image.png?time=1756868400&token=d0bc052c573c657d03764654d9e7cdd55c8ceeb8c66a0d98cd88d4e8607eb941&role=free)

# **开发环境安装**                                                              

`General Driver for Robots` 驱动板的主控模组是 `ESP32`，所以我们在 `Arduino IDE` 开发环境中需要安装好 `ESP32` 对应的开发板。由于官方提供的开发环境安装包是基于`Windows`系统下，直接添加进树莓派5中的`Arduino`文件夹中会提示无法打开`esptool.py`这是因为在`Windows`系统下，`.py`被封装成了`.exe`文件，而在树莓派的`Linux`系统中是无法直接打开`.exe`文件的，因此我们需要在树莓派中将无法打开的`.exe`文件替换为`.py`文件，这其中涉及到文件的封装，过程较为复杂，这里我将需要替换的文件打包整理好了，大家下载之后按照下面教程来就行。

下载好`[packages](https://0.0.0.111)`文件夹，按下面路径：

`/home/username(我的树莓派名称为appendix41)/.arduino15`

将`packages`直接粘贴到该路径下（`packages`中的.py文件我已经给大家配置好了，大家直接用）

![image.png](https://tc-cdn.flowus.cn/oss/057421bd-866c-45a5-958e-c6feadcb587d/image.png?time=1756868400&token=eca3b08d6bf8b6a612737fa8474ce21247e70c52b934173124e89713978e31ae&role=free)

此时可以打开`arduino`可以发现，在开发板选项中多了`ESP32`这一栏，说明开发环境已经成功安装。

按照下图所述，我们选择开发板`ESP32 Dev Module`，作为我们的开发环境

![image.png](https://tc-cdn.flowus.cn/oss/0ff26946-c99f-4c42-90f6-b346fa156d33/image.png?time=1756868400&token=66404dc9b586641c44da5ad4eb826424a03c38f3e174d899198c55262acef39b&role=free)

# **依赖库安装**                                                              

下载[依赖库](https://0.0.0.111)文件，文件名为`libraries`，并进行解压缩，在树莓派5中的文件管理器中找到该路径：

`/home/username/Arduino`

将`libraries`文件粘贴到该路径下，如图所示，检查依赖库是否被`arduino`识别，若`libraries`中的文件均在加载库中显示，则证明依赖库安装成功

![image.png](https://tc-cdn.flowus.cn/oss/6dfcc0ef-fb38-478f-8227-8db734371bf9/image.png?time=1756868400&token=b2b0a08298d239c4a8e25c48fa4d609f9831bc30f03fe4b2646fd1b4c192f373&role=free)

# IMU数据读取                                                             

`General Driver for Robots`板载`9轴IMU`，可读取驱动板自身位置数n据，`[代码](https://0.0.0.111)`在这里给大家修改并整理好了，大家直接用。首先，找一根usb线，按照图示将树莓派5与esp32开发板连接

![image.png](https://tc-cdn.flowus.cn/oss/1c45ab7f-d92c-4e72-9cfc-20e56e2532ba/image.png?time=1756868400&token=f625241fb56f1ee7c34e96528f5ec559d85f753b42cdb12fa0afc14b78024ca8&role=free)

找到并选择端口

`/dev/ttyUSB0`

![image.png](https://tc-cdn.flowus.cn/oss/a1d92cff-b177-4b52-b50b-e74c0c2200e4/image.png?time=1756868400&token=80632cc22bda942890d2f5b37ff949153d089c103cbcd55ce7318522b5f9c3c2&role=free)



