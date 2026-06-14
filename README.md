# FMO 伴侣 M5 Core 适配版

让 M5Stack Core 一代成为一个可以长期摆放、实时监控和守听的 FMO 远程伴侣。

本项目是面向 M5Stack Core Basic / M5Stack Core 一代硬件的 FMO 伴侣固件，针对 M5 Core 的屏幕、实体按键、菜单层级、网络配置和长期守听场景进行了专项适配和优化。项目目标不是替代 FMO 主机，而是让 M5 Core 通过 WiFi 或远程网络连接 FMO，作为一个小屏伴侣终端显示当前通联、站点、QSO 信息，并提供守听和状态提示能力。

## V2.0.0

V2.0.0 是面向 M5 Core 一代的新版固件，重点重构了按键交互、设置菜单和多连接配置能力。

Windows 用户推荐下载 GitHub Release 中的：

- `fmo-companion-m5core-windows-flasher-v2.0.0.zip`

完整解压后运行 `flash_m5core.bat`，按提示输入设备管理器中看到的 COM 口即可刷写。

## 项目定位

FMO 是面向业余无线电爱好者的互联网模拟通联系统。本项目希望把 FMO 的状态监控、守听和常用操作放到一个独立的 M5 Core 小终端上，让它可以放在桌面、台站旁或远程值守环境中持续工作。

本项目的主要工作包括：

- 适配 M5 Core 一代 320x240 屏幕；
- 使用正面 A/B/C 三个实体按键完成主要操作；
- 重构设置菜单，适合无触摸或弱触摸场景；
- 支持 5 组 FMO 连接配置；
- 支持本地 IP、远程地址和 DDNS 动态域名接入；
- 增加连接配置选择、保存、启用和同步状态显示；
- 优化 WiFi、FMO、QSO、站点、音频、省电和待机显示逻辑；
- 保留并优化 LED 通联提示能力。

## 主要功能

### 首页监控

- 当前通联呼号大字体显示；
- 上次通联呼号显示；
- 当前站点显示；
- QSO 数量显示；
- WiFi 连接状态图标；
- 电池状态图标；
- 底部状态栏；
- 静音、设置、省电入口。

### M5 Core 按键操作

- A / C：左右移动、切换选项或调节数值；
- B：确认、进入、保存或启用；
- 设置页内保持清晰焦点，高亮当前选中项；
- 适配 M5 Core 一代屏幕比例，避免 90/270 度旋转造成布局失配。

### 多连接配置

V2.0.0 支持 5 组连接配置：

- 配置1默认保留占位 WiFi/FMO 信息；
- 配置2-5默认为空，可由用户自行配置；
- 每组配置包含 WiFi、FMO 地址、DDNS 远程开关、QSO/服务器同步状态；
- 可以在设置页选择当前配置；
- 可以进入网络配置修改当前选中配置；
- 可以保存配置并启用为当前连接配置。

FMO 地址支持填写：

- 局域网 IP，例如 `192.168.31.146`；
- 带端口的地址，例如 `192.168.31.146:8080`；
- DDNS 动态域名，例如 `example.ddns.net`。

### 全局设置

- 全局呼号设置；
- 音量设置；
- 背光设置；
- 屏幕旋转：正常 / 180 度；
- 深色 / 浅色主题；
- 低亮省电开关；
- 保存重启。

休眠和省电时钟页显示的呼号来自全局“呼号设置”，不跟随某一组连接配置变化。

### FMO 与 QSO

- 通过 WebSocket 获取音频、事件、站点、QSO 数据；
- 支持手动同步 QSO/服务器；
- 当前站点显示与刷新；
- QSO 数量显示；
- 已通联、新呼号、当天未通联呼号可用于 LED 提示逻辑。

### 音频与守听

- ESP32 内置 DAC 输出；
- 默认静音，用户可手动开启；
- 音量可在设置页调节；
- 适合作为桌面守听终端。

### LED 提示

支持连接在 GPIO15 的 10 颗 WS2812 / GRB LED 灯带：

- 普通通联：绿色常亮；
- 本机呼号通联：红色常亮；
- 从未通联过的新呼号：彩虹色呼吸提醒；
- 当天尚未通联的老呼号：橙色呼吸提醒；
- 近期重复出现的呼号：蓝色提醒。

普通不带 LED 灯带的 M5 Core 也可以正常使用。

## 硬件

当前主要适配：

- M5Stack Core Basic；
- M5Stack Core 一代兼容硬件；
- ESP32 + ILI9341 兼容 320x240 LCD；
- M5 正面 A/B/C 三个实体按键；
- ESP32 内置 DAC 音频输出；
- 可选 GPIO15 WS2812 LED 灯带。

## 默认配置

发布固件默认：

- 固件显示版本：`v2.0.0`；
- 默认呼号：`BH1JSS`；
- 默认 WiFi SSID：`YOUR_WIFI_SSID`；
- 默认 WiFi 密码：`YOUR_WIFI_PASSWORD`；
- 默认 FMO 地址：`192.168.31.146`；
- 默认启用配置1；
- 配置2-5为空槽。

这些信息都可以在设备设置页中修改。

## 下载文件说明

GitHub Release 中通常包含：

- `fmo-companion-m5core-windows-flasher-v2.0.0.zip`：推荐的 Windows 一键刷写包；
- `fmo-companion-m5core-firmware-v2.0.0.zip`：固件 bin 与 SHA-256 校验文件；
- `fmo-companion-m5core-merged-v2.0.0.bin`：完整固件，包含 bootloader、分区表和 app，从 `0x0` 地址刷写；
- `fmo-companion-m5core-app-only-v2.0.0.bin`：仅 app 分区固件，从 `0x10000` 地址刷写，适合熟悉 ESP-IDF 的用户。

推荐普通用户使用 Windows 刷写包或完整 merged 固件。

## Windows 刷写

1. 下载并完整解压 `fmo-companion-m5core-windows-flasher-v2.0.0.zip`。
2. 用 USB 连接 M5 Core。
3. 打开 Windows 设备管理器，查看 COM 口，例如 `COM5`。
4. 双击运行 `flash_m5core.bat`。
5. 输入 COM 口并回车。
6. 等待出现 `Hash of data verified.` 和 `[OK] Flash completed.`。

如果刷写失败，可以重新插拔 USB 后运行 `flash_m5core_slow.bat`。

## 源码编译

本项目使用 ESP-IDF 5.5.x。

```sh
. /path/to/esp-idf/export.sh
idf.py build
idf.py -p /dev/tty.usbserial-xxxx -b 115200 flash
```

如需生成完整 merged 固件，可使用 ESP-IDF 的 `esptool.py merge_bin`，参考 `build/flash_args` 中的地址和参数。

## 项目关系

本项目的早期工作参考并基于 [zhaozhengde/esp32-fmo-companion](https://github.com/zhaozhengde/esp32-fmo-companion) 原项目，在此向原作者和 FMO 生态中的开发者、测试者表示感谢。

V2.0.0 起，本项目作为 M5 Core 定向适配版独立维护，目标是把 M5 Core 打造成一个更易用、更稳定、更适合长期运行的 FMO 远程伴侣终端。

## 许可证

本项目延续原项目许可证，详见 [LICENSE](LICENSE)。
