# Tern Font Bridge

非官方 LSPosed 原生模块，通过替换 Tern 内嵌字体源，让默认普通字体使用 **Maple Mono Normal NF CN Regular**。

Unofficial LSPosed native module replacing Tern’s bundled Regular font source with Maple Mono NF CN. This is **not** a general-purpose font registration API.

## 状态 / Status

- 原型 `local.tern.diagnostic` 0.3 已在用户设备验证：冷启动直接生效，中文正常，设置页默认字体名称变为 Maple。
- 此仓库使用新包名 `io.github.particleg.ternfontbridge`；重命名版本仅完成构建，尚未进行设备运行验证。
- 实验性软件；仅替换 BerkeleyMono Regular。Medium/粗体未替换，真正的 Italic 未嵌入，部分 UI 符号仍可能缺失。
- 无网络权限，不修改 Tern APK、系统字体或 SELinux。

## 兼容性

仅支持以下 **Tern 0.9.5 arm64** APK，Java 入口严格检查完整 SHA-256：

```
20d357f2a3a3a1e58ed207244f9148fe4ff7236059ed27a111cfb5704d1598aa
```

版本字符串相同但 APK 哈希不同也会跳过。更新 Tern 需要重新适配，不能只修改哈希绕过检查。

原型测试环境：LSPosed 2.0.2 (7668)，Zygisk Next 1.5.0，Android API 36，arm64。未验证全部 LSPosed 分支。

## 安装与撤销

1. 保存会话，安装自行构建并签名的 APK。
2. **先禁用旧的 Tern Font Diagnostic 模块**，不要同时启用两者。
3. 启用 Tern Font Bridge，只勾选 `so.stencil.tern`。
4. 强行停止并重开 Tern。原型在字体设置留空时已自动采用 Maple。

撤销：禁用模块，强停并重开 Tern。若手动指定 Maple，请恢复默认字体设置。不要清除应用数据。

作用域限制不等于安全沙箱：模块在目标进程执行原生代码，可能导致崩溃、卡死或未保存状态丢失。尚未完成长期稳定性测试。

## 构建（Termux / ARM64 Android）

当前脚本专用于 Termux，并依赖 Android 系统库；不是通用 Linux/NDK 或 Gradle 构建。

```sh
pkg install clang python openjdk-21 d8 aapt aapt2 apksigner curl
bash fetch-deps.sh
bash build.sh
```

产物：`build/tern-font-bridge-0.3.0-preview-unsigned.apk`。原生库以 ZIP_STORED 存储，按 16 KB 对齐。

依赖 JAR 的 SHA-256 固定在 `deps.sha256`。旧版 Android stub JAR 仅用于编译和资源打包，不打包进模块；运行最低版本声明为 API 28，不代表所有 API 28 设备已验证。

### 签名

仓库不包含签名密钥，也不自动生成正式密钥。请在仓库外创建并备份自己的密钥，交互输入密码：

```sh
keytool -genkeypair -keystore /your/private/path/release.p12 -storetype PKCS12 -alias release -keyalg RSA -keysize 3072 -validity 10000
apksigner sign --ks /your/private/path/release.p12 --ks-key-alias release --out build/tern-font-bridge-0.3.0-preview.apk build/tern-font-bridge-0.3.0-preview-unsigned.apk
apksigner verify --verbose build/tern-font-bridge-0.3.0-preview.apk
```

不要把私钥、密码或原型测试密钥提交到 Git。更换包名会并行安装；同包名换签名通常无法覆盖升级。

## 实现

通过 LSPosed `native_init` 接收库加载通知，检查目标 ELF 范围、getter 指令及 vtable，然后临时调整只读页权限，原子替换一个方法指针并恢复页面保护。不会 inline patch 原始代码。

只匹配库内偏移 `0xD0535B`、长度 `2288420` 的原始字体源；返回模块只读段中的 Maple 字节，其他字体源返回原值。ARM64 汇编入口保留参数寄存器并恢复返回值。字体随模块驻留，不读取外部字体路径。

日志标签仍为 `TernFontProbe`，仅限量记录技术元数据，不记录聊天文本。重复加载通知不会重复安装。

## 字体与许可证

`native/Maple-Regular.ttf` 为用户提供的 `MapleMonoNormal-NF-CN-unhinted.zip` 中原始 Regular 文件，未修改。归档的上游发布版本号尚未确认；以文件哈希标识，不声称是某个特定 release。

- 上游：https://github.com/subframe7536/maple-font
- 字体 SHA-256：`9ae0c348a44a681fa8fa20a34e782e95316cec880102295a128b65760944c0f5`
- 字体许可：SIL OFL 1.1，见 `assets/licenses/MapleMono-OFL.txt`（同样打包进 APK）。
- 模块源码采用 [MIT License](LICENSE)，Copyright (c) 2026 ParticleG；Maple 字体独立遵守 OFL 1.1，不适用 MIT。

不包含 Tern APK、libtern.so 或其内嵌商业字体。本项目不隶属于 Tern、Stencil、LSPosed 或 Maple Mono。

## 参考

- https://github.com/LSPosed/LSPosed/wiki/Native-Hook
- https://github.com/LSPosed/LSPosed/wiki/Module-Scope
