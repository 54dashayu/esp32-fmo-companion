# 基于ESP32开发板的FMO伴侣

本 Fork 基于 [zhaozhengde/esp32-fmo-companion](https://github.com/zhaozhengde/esp32-fmo-companion) 原项目继续开发，当前主要适配 M5Stack Core Basic、M5 Fire 等设备。

## M5 Core Fork v0.2.0

当前固件显示版本为 **v1.2.3**。

Windows 用户推荐从 [M5 Core Windows 刷写包 v0.2.0](https://github.com/54dashayu/esp32-fmo-companion/releases/tag/m5core-flasher-v0.2.0) 下载 `fmo-companion-m5core-windows-flasher-v0.2.0.zip`，完整解压后运行 `flash_m5core.bat`。

### Bug 修复

#### 设置菜单操作修复

- 修复音量和屏幕亮度无法通过 M5 正面 A/B/C 三键调节的问题；
- 进入音量或亮度页面后，滑块会自动获得焦点；
- 按中间键进入调节模式，滑块显示橙色高亮边框；
- 使用左右键调节数值，每次增加或减少 5；
- 再次按中间键确认并退出调节模式；
- 离开页面时自动退出调节状态，避免按键操作异常。

#### 启动及 Windows 刷写修复

- Windows 刷写工具使用完整 merged 固件，从 `0x0` 地址开始刷写；
- 修复只将应用程序 bin 刷入错误地址后，设备黑屏、无法启动的问题；
- 刷写包内置便携 Python 和 esptool，不需要另外安装开发环境；
- 提供普通刷写脚本和低速刷写脚本，改善部分 USB 或供电不稳定设备的刷写成功率。

#### 状态栏中文显示修复

- 修复未连接 WiFi 时，底部状态栏中“连接”等中文显示为乱码方框的问题；
- WiFi、FMO 和其他底部状态提示统一使用完整中文字体。

### 新增功能

#### 通联状态 LED 提示

支持连接在 **GPIO15** 的 **10 颗 WS2812/GRB LED 灯带**：

- 接收到普通呼号通联时，LED 灯带持续亮绿色；
- 本机呼号正在通联时，LED 灯带持续亮红色；
- 通联结束后，LED 灯带自动关闭；
- 呼号提醒动画结束后，如果通联仍在继续，自动恢复红色或绿色通联状态灯。

#### 新呼号 LED 提醒

固件会将收到的呼号与同步后的通联记录进行比对：

- **从未通联过的呼号**：彩虹色呼吸灯闪烁 2 秒；
- **以前通联过，但当天尚未通联的呼号**：橙色呼吸灯闪烁 2 秒；
- **最近 15 分钟内又重新出现的已通联呼号**：蓝色呼吸灯闪烁 2 秒。

呼号比较会自动忽略大小写和多余空格。
设置中的本机呼号不会触发上述新呼号动画，本机通联时仅显示红色常亮提示。

#### 默认设置更新

- 默认本机呼号修改为 `BH1JSS`；
- 默认 FMO 地址修改为 `192.168.31.146`；
- WebSocket 音频、事件和站点地址会根据 FMO 地址自动生成；
- 如果设备中保存的仍是旧默认呼号 `BI8SIG` 或旧默认地址 `192.168.3.165`，升级后会自动迁移到新默认值；
- 用户手动修改过的其他呼号或 FMO 地址不会被覆盖。

### 下载文件说明

- `fmo-companion-m5core-windows-flasher-v0.2.0.zip`：推荐 Windows 刷写工具包；
- `fmo-companion-m5core-simple-windows-flasher-v0.2.0.zip`：简化版 Windows 刷写工具包；
- `fmo-companion-m5core-merged-v0.2.0.bin`：包含 Bootloader、分区表和应用程序的完整固件，应从 `0x0` 地址刷写；
- `fmo-companion-m5core-app-only-v0.2.0.bin`：仅应用程序固件，只适合熟悉 ESP-IDF 分区和刷写地址的用户；
- `fmo-companion-m5core-firmware-v0.2.0.zip`：包含完整固件、app-only 固件和 SHA-256 校验文件。

完整 merged 固件 SHA-256：

`f340977c5cea933b9c8e90006cd048d402bd49f99cf8c063a6a42f94495e90cc`

### 注意事项

- 刷写前请完整解压 Windows 刷写工具包；
- 刷写过程中请勿拔掉 USB；
- LED 提示功能需要设备具有兼容的 GPIO15 WS2812 灯带；
- 普通不带 LED 灯带的设备仍可正常使用其他功能。

---

## 项目背景

FMO，即 **NFM over Internet**，可以理解为“互联网模拟通联”。
它是一种通过互联网承载 NFM 模拟对讲体验的通信方式，服务于广大的业余无线电爱好者。

FMO 主机由 **BG5ESN** 开发，是一套面向业余无线电爱好者设计的互联网对讲硬件系统。

---

## FMO 伴侣的开发目的

随着使用 FMO 的台友越来越多，在实际使用过程中，很多台友也开发了相关的仪表盘，web端、app端，大家都希望它扩展FMO的使用周边。

因此，本项目尝试基于常见的 ESP32 开发板，制作一个 **FMO 伴侣设备**。

FMO 伴侣并不替代 FMO 主机本身，而是作为 FMO 主机的辅助终端存在。它通过 WiFi 连接到 FMO 服务端，并通过 WebSocket 获取音频、事件、站点和 QSO 信息，从而实现：

- 当前通联呼号显示；
- 通联状态提示；
- 当前站点显示；
- 上次通联记录显示；
- QSO 数量显示与同步；
- 收藏站点切换；
- WiFi 与 FMO Host 设置；
- 电池电量显示；
- 低电量省电；
- 待机时钟；
- 触摸屏交互；
- 本机音频播放控制。

这个项目的初衷，是让 FMO 仪表盘功能在另一个硬件平台实现的可能性，选用的也是性能较弱的ESP32开发板，目的在于希望使用低廉的硬件实现仪表盘功能的可能性。

FMO 伴侣提供的一切数据均来源于FMO主机接口。

---

## 致谢

本项目的开发离不开 FMO 作者 **BG5ESN** 的巨大贡献， 正是因为有了 FMO 这一开放、独立、去中心化的互联网模拟通联系统，才有了本项目进一步扩展和探索的可能。

同时，也特别感谢以 **BI8SPD** 和台友在项目开发和测试过程中提供的支持、建议和反馈。

---

## 开源与继续开发

本项目选择开源，是希望更多业余无线电爱好者、嵌入式开发者和 FMO 用户能够参与进来。
本项目使用 ESP-IDF v5.5.1 开发，UI 基于 LVGL。当前 fork 正在移植到 M5Stack Core Basic，显示屏走 ILI9341 兼容驱动，使用正面 A/B/C 三个实体按键替代原项目的触摸输入，音频默认使用 ESP32 内置 DAC 输出。目前限于硬件本身性能，蓝牙部分完成了初始的代码工作，但无法在当前适配的开发使用。
你可以基于本项目继续开发：

- 适配不同 ESP32 开发板；
- 适配不同尺寸的 LCD；
- 改进 LVGL UI；
- 增加实体按键；
- 增加旋钮控制；
- 增加外置 I2S DAC；
- 增加蓝牙音频；
- 增加外壳结构设计；
- 增加更多 FMO 控制功能；
- 优化低功耗；
- 移植到 ESP32-S3 / ESP32-C3 / ESP32-P4 等平台。

希望这个项目也能成为 FMO 生态中的一个小小扩展，让更多台友可以根据自己的需求制作属于自己的 FMO 伴侣设备。

欢迎大家继续改进、移植、复刻和二次开发。



## 目录

-   [项目简介](#%E9%A1%B9%E7%9B%AE%E7%AE%80%E4%BB%8B)
-   [功能特性](#%E5%8A%9F%E8%83%BD%E7%89%B9%E6%80%A7)
-   [硬件配置](#%E7%A1%AC%E4%BB%B6%E9%85%8D%E7%BD%AE)
-   [GPIO 引脚分配](#gpio-%E5%BC%95%E8%84%9A%E5%88%86%E9%85%8D)
-   [软件架构](#%E8%BD%AF%E4%BB%B6%E6%9E%B6%E6%9E%84)
-   [启动流程](#%E5%90%AF%E5%8A%A8%E6%B5%81%E7%A8%8B)
-   [配置说明](#%E9%85%8D%E7%BD%AE%E8%AF%B4%E6%98%8E)
-   [WebSocket 说明](#websocket-%E8%AF%B4%E6%98%8E)
-   [音频说明](#%E9%9F%B3%E9%A2%91%E8%AF%B4%E6%98%8E)
-   [QSO 同步说明](#qso-%E5%90%8C%E6%AD%A5%E8%AF%B4%E6%98%8E)
-   [省电模式](#%E7%9C%81%E7%94%B5%E6%A8%A1%E5%BC%8F)
-   [电池监测](#%E7%94%B5%E6%B1%A0%E7%9B%91%E6%B5%8B)
-   [M5 Core 页面切片](docs/m5_core_page_slices.md)
-   [编译与烧录](#%E7%BC%96%E8%AF%91%E4%B8%8E%E7%83%A7%E5%BD%95)
-   [许可证](#%E8%AE%B8%E5%8F%AF%E8%AF%81)



## 项目简介

项目基于BG5ESN开发的FMO

它通过 WiFi 连接到 FMO 主机，并通过 WebSocket 接收：

-   网络音频流；
-   通联事件；
-   当前站点信息；
-   站点列表；
-   QSO 列表。

设备通过 LVGL 图形界面显示当前呼号、上次通联、当前站点、QSO 数量、WiFi 状态、电池状态等信息。



## 功能特性

### UI 显示

-   当前通联呼号大字体显示；
-   RX 状态提示；
-   当前站点显示；
-   上次通联呼号显示；
-   QSO 数量显示；
-   WiFi 信号状态图标；
-   电池状态图标；
-   顶部日期时间显示；
-   底部状态栏；
-   待机时钟页面；
-   省电时钟页面；
-   设置页面。

### 网络功能

-   WiFi STA 连接；
-   WiFi 断线重连；
-   WiFi RSSI 周期刷新；
-   WiFi 扫描；
-   FMO Host 配置；
-   自动生成 WebSocket URL。

### WebSocket 功能

-   Audio WebSocket：接收 PCM 音频；
-   Event WebSocket：接收通联事件；
-   Station WebSocket：获取当前站点、站点列表、QSO 列表；
-   QSO 手动同步；
-   QSO 增量检查。

### 音频功能

-   开机默认静音；
-   用户手动开启音频；
-   ESP32 内置 DAC 输出；
-   音量设置；
-   音频缓冲与自适应缓冲；
-   蓝牙音频输出预留。

### 电源与省电

-   电池 ADC 采样；
-   电池百分比估算；
-   低电量提醒；
-   低电量自动进入省电模式；
-   手动省电模式；
-   背光亮度设置；
-   省电模式降低背光并关闭 WiFi。



## 硬件配置

当前 fork 默认适配 M5Stack Core Basic / Basic v2.7 一类硬件：

| 功能    | 配置            |
| ----- | ------------- |
| 主控    | ESP32-D0WDQ6-V3 / ESP32 |
| 显示屏   | ILI9342C，按 ILI9341 兼容驱动初始化 |
| 分辨率   | 320 × 240     |
| 输入    | 正面 A/B/C 三个实体按键 |
| UI 框架 | LVGL          |
| 音频输出  | ESP32 内置 DAC，GPIO25 |
| 电池检测  | ADC，GPIO35 |
| 存储    | NVS           |
| 可选存储  | SD 卡 / SPIFFS |



## GPIO 引脚分配

GPIO 配置位于：

main/board\_config.h

M5Stack Core Basic 引脚如下：

| 功能          | GPIO   |
| ----------- | ------ |
| LCD CS      | GPIO14 |
| LCD DC      | GPIO27 |
| LCD MOSI    | GPIO23 |
| LCD MISO    | GPIO19 |
| LCD SCLK    | GPIO18 |
| LCD BL      | GPIO32 |
| LCD RST     | GPIO33 |
| Button A    | GPIO39 |
| Button B    | GPIO38 |
| Button C    | GPIO37 |
| Audio DAC   | GPIO25 |
| Audio EN    | 未连接 |
| SD CS       | GPIO4  |
| SD SCLK     | GPIO18 |
| SD MISO     | GPIO19 |
| SD MOSI     | GPIO23 |
| Battery ADC | GPIO35 |

按键映射：Button A 为上一个/向左，Button B 为确认，Button C 为下一个/向右。当前阶段优先保证屏幕点亮和基础导航，后续还需要继续按 M5 无触摸交互优化设置页。



## 软件架构

主要模块如下：
```text
main.c
  ├── app\_config.h          应用默认配置与功能开关
  ├── board\_config.h        板级 GPIO 与硬件参数
  ├── app\_settings.c        NVS 配置管理
  ├── app\_ui.c              LVGL 主界面、设置页、待机时钟
  ├── ui\_async.c            后台任务安全更新 UI
  ├── wifi\_manager.c        WiFi 连接、重连、扫描
  ├── audio\_ws.c            WebSocket、网络音频、QSO 同步
  ├── audio\_output.c        音频输出抽象层
  ├── bt\_audio.c            蓝牙音频输出预留
  ├── event\_parser.c        Event/QSO JSON 解析
  ├── station\_parser.c      Station JSON 解析与请求构造
  ├── battery\_monitor.c     电池采样与低电处理
  ├── app\_power\_save.c      省电模式管理
  ├── lv\_port\_disp.c        LVGL 显示移植
  ├── lv\_port\_indev.c       LVGL 输入移植
  ├── ili9341.c             M5Stack LCD 兼容显示驱动
  └── lv_port_indev.c       M5Stack A/B/C 按键输入

```

## 启动流程

系统启动入口：

app\_main()

启动流程：

1\. 初始化 NVS
2\. 加载应用配置 app\_settings
3\. 初始化省电管理 app\_power\_save
4\. 初始化 LVGL
5\. 初始化 LCD 显示
6\. 初始化触摸输入
7\. 创建 LVGL tick 定时器
8\. 创建 UI
9\. 初始化音频输出层
10\. 启动 WiFi
11\. WiFi 获取 IP 后启动 WebSocket
12\. 启动电池监测任务
13\. 进入 LVGL 主循环



## 配置说明

默认配置位于：

main/app\_config.h

常用配置项：

#define APP\_VERSION\_TEXT "v1.2.1"

#define APP\_DEFAULT\_OWNER\_CALLSIGN "BI8SIG"

#define DEFAULT\_WIFI\_SSID       "YOUR\_WIFI\_SSID"
#define DEFAULT\_WIFI\_PASSWORD   "YOUR\_WIFI\_PASSWORD"

#define DEFAULT\_FMO\_HOST        "192.168.3.165"

#define DEFAULT\_AUDIO\_VOLUME       60
#define DEFAULT\_BACKLIGHT\_PERCENT  30

#define DEFAULT\_IDLE\_IMAGE\_ENABLED     1
#define DEFAULT\_IDLE\_IMAGE\_TIMEOUT\_MS  150000

配置会保存到 NVS：

namespace: appcfg
key: settings

配置结构体版本：

#define APP\_SETTINGS\_VERSION 9

修改 `app_settings_t` 结构体后，应提升 `APP_SETTINGS_VERSION`。



## WebSocket 说明

用户只需要配置 FMO Host，例如：

192.168.3.165

程序会自动生成：

ws://192.168.3.165/audio
ws://192.168.3.165/events
ws://192.168.3.165/ws

对应关系：

| 功能                 | 路径      |
| ------------------ | ------- |
| 音频 WebSocket       | /audio  |
| 事件 WebSocket       | /events |
| 站点 / QSO WebSocket | /ws     |



## 音频说明

服务端音频格式：

raw 16-bit PCM little-endian
mono
8000 Hz

默认配置：

#define AUDIO\_WS\_INPUT\_SAMPLE\_RATE 8000
#define AUDIO\_OUTPUT\_SAMPLE\_RATE   32000
#define AUDIO\_UPSAMPLE\_FACTOR      4

即：

8000 Hz 输入
  ↓
4 倍升采样
  ↓
32000 Hz DAC 输出

当前设计：

-   开机默认静音；
-   不自动连接 `/audio`；
-   用户点击静音按钮后才开启 Audio WebSocket；
-   播放任务等待缓冲达到阈值后再打开功放。



## 蓝牙音频

项目预留蓝牙 A2DP Source 输出：

audio\_output.c
bt\_audio.c

默认关闭：

#define APP\_CONFIG\_BT\_AUDIO\_ENABLE 0

如需启用：

#define APP\_CONFIG\_BT\_AUDIO\_ENABLE 1
#define APP\_CONFIG\_BT\_TARGET\_NAME "YOUR\_BT\_SPEAKER\_NAME"

> ESP32-WROOM-32E-N4 内存较紧张，启用蓝牙音频可能增加内存压力。



## QSO 同步说明

QSO 同步由以下模块处理：

audio\_ws.c
event\_parser.c

### 无缓存时

首次启动如果没有 QSO 缓存：

不自动完整扫描
提示用户手动同步

### 有缓存时

周期性请求最新一条记录：

page=0
pageSize=1

如果 latestLogId 变化，则启动增量扫描。

### 手动完整同步

设置页点击 QSO 同步后，从第 0 页开始扫描。

默认参数：

#define QSO\_COUNT\_FULL\_PAGE\_SIZE       20
#define QSO\_COUNT\_MAX\_FULL\_SCAN\_PAGES  500

最多扫描：

20 × 500 = 10000 条记录



## 省电模式

省电模块：

app\_power\_save.c

支持的省电原因：

APP\_POWER\_SAVE\_REASON\_MANUAL
APP\_POWER\_SAVE\_REASON\_LOW\_BATTERY

进入省电模式时：

1\. 停止 WebSocket
2\. 停止 WiFi
3\. UI 进入省电时钟页
4\. 背光降低

退出省电模式时：

1\. 退出省电时钟页
2\. 恢复背光
3\. 重启 WiFi
4\. WiFi 获取 IP 后恢复 WebSocket

省电相关配置：

#define APP\_POWER\_SAVE\_BACKLIGHT\_PERCENT 10

#define APP\_POWER\_SAVE\_ENTER\_PERCENT 10
#define APP\_POWER\_SAVE\_EXIT\_PERCENT  25

#define APP\_POWER\_SAVE\_ENTER\_MV      3450
#define APP\_POWER\_SAVE\_EXIT\_MV       3650



## 电池监测

电池模块：

battery\_monitor.c

功能：

-   ADC1 采样；
-   电压校准；
-   电池百分比估算；
-   UI 电池图标更新；
-   低电量自动进入省电模式。

默认参数：

#define BATTERY\_VOLTAGE\_SCALE\_PERMILLE 2000
#define BATTERY\_VOLTAGE\_OFFSET\_MV      0

#define BATTERY\_PERCENT\_EMPTY\_MV       3000
#define BATTERY\_PERCENT\_FULL\_MV        4180

如果日志电压与万用表测量值存在比例误差，需要调整：

BATTERY\_VOLTAGE\_SCALE\_PERMILLE

如果存在固定偏差，可通过设置页电池偏移校准。



## 编译与烧录

### 1\. 安装 ESP-IDF

请先安装并配置 ESP-IDF 环境。

Linux/macOS：

. $IDF\_PATH/export.sh

Windows PowerShell：

%IDF\_PATH%\\export.ps1



### 2\. 设置目标芯片

idf.py set-target esp32



### 3\. 配置项目

idf.py menuconfig



### 4\. 编译

idf.py build



### 5\. 烧录

Linux/macOS 示例：

idf.py -p /dev/ttyUSB0 flash

Windows 示例：

```
idf.py -p COM5 flash
```
## 许可证

本项目采用 [MIT License](LICENSE) 开源。

你可以自由使用、修改和分发本项目代码，但需要保留原作者版权声明和许可证文本。
