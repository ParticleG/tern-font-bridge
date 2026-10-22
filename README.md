# Tern Font Bridge

非官方 LSPosed 原生模块，通过替换 Tern 内嵌字体源，让默认普通字体使用 **Maple Mono Normal NF CN Regular**。

Unofficial LSPosed native module replacing Tern’s bundled Regular font source with Maple Mono NF CN. This is **not** a general-purpose font registration API.

## 状态 / Status

- 原型 `local.tern.diagnostic` 0.3 已在用户设备验证：冷启动直接生效，中文正常，设置页默认字体名称变为 Maple。
- 新包名 `io.github.particleg.ternfontbridge` 的 Actions/NDK 构建、正式签名 dev APK 已在禁用旧模块后通过设备冷启动验证，中文正常显示。该验证不代表长期稳定性或所有字重、符号均正常。
- 实验性软件；仅替换 BerkeleyMono Regular。Medium/粗体未替换，真正的 Italic 未嵌入，部分 UI 符号仍可能缺失。
- 无网络权限，不修改 Tern APK、系统字体或 SELinux。

## 兼容性

仅支持以下 **Tern 0.9.5 arm64** APK，Java 入口严格检查完整 SHA-256：

```
20d357f2a3a3a1e58ed207244f9148fe4ff7236059ed27a111cfb5704d1598aa
```

版本字符串相同但 APK 哈希不同也会跳过。更新 Tern 需要重新适配，不能只修改哈希绕过检查。

原型测试环境：LSPosed 2.0.2 (7668)，Zygisk Next 1.5.0，Android API 36，arm64。未验证全部 LSPosed 分支。

## v0.3.0 发布范围

首个公开版本保留已验证的 Regular-only 实现。正式包与已测试 dev 包使用相同签名，versionCode 从 1 升至 6002，可覆盖升级；旧包名原型不能覆盖升级，必须保持禁用。

## 安装与撤销

1. 保存会话，从 [Releases](https://github.com/ParticleG/tern-font-bridge/releases) 下载签名 APK，或自行构建并签名。
2. **先禁用旧的 Tern Font Diagnostic 模块**，不要同时启用两者。
3. 启用 Tern Font Bridge，只勾选 `so.stencil.tern`。
4. 强行停止并重开 Tern。原型在字体设置留空时已自动采用 Maple。

撤销：禁用模块，强停并重开 Tern。若手动指定 Maple，请恢复默认字体设置。不要清除应用数据。

作用域限制不等于安全沙箱：模块在目标进程执行原生代码，可能导致崩溃、卡死或未保存状态丢失。尚未完成长期稳定性测试。

## 构建（Termux / ARM64 Android）

本节的 `build.sh` 专用于 Termux，并依赖 Android 系统库；云端使用独立的 `scripts/ci-build.sh` 和标准 Android SDK/NDK。

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

## GitHub Actions 自动签名与发布

工作流使用 Ubuntu、NDK 27.2.12479018、Android SDK 35 / Build Tools 35.0.0 与 Java 21。第三方 Actions 固定到提交 SHA。

- 推送 `vX.Y.Z`：构建正式签名 APK、上传 Actions artifact，创建正式 Release 并附加 APK / SHA256SUMS。
- 推送 `preX.Y.Z`：同上，但创建 Prerelease（不标记 latest）。
- 手动 workflow_dispatch：生成正式签名的 dev APK artifact，不创建 Release。
- tag 使用严格数字格式，不接受后缀或前导零；每段 0–999。
- versionCode = `(major * 1000000 + minor * 1000 + patch) * 2 + stage`，预发布 stage=1，正式 stage=2。同版本预发布可升级正式版。dev 构建 code=1，仅供测试。
- 重跑已有发布标签不会覆盖现有 Release；需要新版本请推送新 tag。

仓库 Secrets：ANDROID_KEYSTORE_BASE64、ANDROID_KEYSTORE_PASSWORD、ANDROID_KEY_ALIAS、ANDROID_KEY_PASSWORD。
仓库变量 ANDROID_CERT_SHA256 用于校验签名证书。密钥只在签名步骤解码到 runner 临时目录，用后删除，不进入 artifact。

正式证书 SHA-256：`52468c95a3e1c6cbd766eec99620575aafb0f72c249c945d1c447a409b004889`。

请限制仓库写权限并保护发布标签：可修改工作流的人可能滥用签名 Secrets。GitHub Secrets 不是可导出的密钥备份，应另行安全备份发布密钥。
