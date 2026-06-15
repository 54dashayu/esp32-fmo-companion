# TF 卡导入配置文件

V2.0 支持从 TF 卡读取一个 CSV 配置文件。这个文件包含全局呼号和 5 组网络配置，适合在电脑上先编辑好，再插入 M5 Core 读取。

## 文件名

文件名固定为：

- `fmo_profiles.csv`

文件必须放在 TF 卡根目录。进入 M5 的设置页，选中第一行 `TF卡配置文件` 后按中键读取。导入成功后配置会保存到 NVS。

## CSV 格式

```csv
呼号,配置号,配置名(英文),WiFi名称,WiFi密码,FMO地址,DDNS远程,启用
BH1JSS,1,Home,HomeWiFi,HomePassword,192.168.31.146,0,1
BH1JSS,2,Car,CarWiFi,CarPassword,myfmo.ddns.net:8080,1,0
BH1JSS,3,Station,StationWiFi,StationPassword,10.0.0.20,0,0
BH1JSS,4,Backup1,,,,0,0
BH1JSS,5,Backup2,,,,0,0
```

仓库内也提供了范例文件：[fmo_profiles.example.csv](fmo_profiles.example.csv)。复制到 TF 卡根目录时，把文件名改成 `fmo_profiles.csv`。

第一行表头可以使用中文，M5 会跳过第一行，不会把表头显示到设备上；但后续 5 行的列顺序必须保持不变。

字段说明：

| 字段 | 说明 |
| --- | --- |
| `owner_callsign` | 全局本机呼号，建议 5 行保持一致 |
| `slot` | 配置槽，取值 `1` 到 `5` |
| `name_ascii` | 配置名称，最长 15 字节；建议只用英文、数字、空格和常用 ASCII 符号 |
| `wifi_ssid` | WiFi 名称，最长 32 字节 |
| `wifi_password` | WiFi 密码，最长 63 字节；开放网络可留空 |
| `fmo_host` | FMO 地址，只填 `Host[:Port]`，不要填 `http://`、`ws://` 或路径 |
| `ddns_remote_enabled` | DDNS/远程标记，`1` 为开启，`0` 为关闭 |
| `active` | 当前启用配置，只建议一行填 `1` |

M5 固件内置中文字体只覆盖界面常用字，不保证显示任意中文配置名。如果 `name_ascii` 中写入中文，导入器会保留默认名称 `配置1` 到 `配置5`，避免屏幕显示方块乱码。

字符集建议：

- 在 Excel 中保存时选择 `CSV UTF-8 (逗号分隔) (*.csv)`。
- 在 Numbers/WPS/文本编辑器中保存时选择 `UTF-8`，最好保存为 `UTF-8 with BOM`。
- 不建议保存为 `ANSI`、`GBK` 或系统默认编码，否则中文表头可能乱码。
- 配置名建议只用英文、数字、空格和常用 ASCII 符号；WiFi 名称、密码和 FMO 地址按实际内容填写。

如果 SSID、密码或配置名称中包含英文逗号，需要用英文双引号包起来：

```csv
BH1JSS,1,"Home,Desk","Home,WiFi","pass,word",192.168.31.146,0,1
```

## 导入规则

- 只读取 TF 卡根目录的 `fmo_profiles.csv`。
- 被标记为 `active=1` 的配置必须同时包含 `wifi_ssid` 和 `fmo_host`。
- 如果没有任何一行写 `active=1`，程序会选择第一组同时包含 `wifi_ssid` 和 `fmo_host` 的配置作为当前配置。
- 文件解析失败时不会覆盖原有 NVS 配置。
- TF 卡未插入、未格式化、文件不存在时，设备继续使用原有配置。

## 建议流程

1. 在电脑上复制 `docs/fmo_profiles.example.csv`。
2. 修改呼号、WiFi、FMO 地址，并保存为 UTF-8 文本。
3. 放到 TF 卡根目录。
4. 文件名改为 `fmo_profiles.csv`。
5. 插入 M5 Core。
6. 进入设置页第一行 `TF卡配置文件`，按 `读取`。
7. 回到设置页确认状态为 `已读取`，并检查当前配置名称和网络信息。
