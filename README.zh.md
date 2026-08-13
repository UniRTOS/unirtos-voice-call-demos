# unirtos-voice-call-demos

中文 | [English](README.md)

本仓库推荐通过 unirtos-cli 的 demo 工作流使用，以保证创建、环境拉取和编译流程一致。

## 功能描述

本 Demo 展示 UniRTOS 上 IMS 语音通话（Voice Call）基础开发流程，覆盖编解码器初始化、来电事件处理、自动接听、主动拨号与挂断控制。

- 演示 ES8311 音频编解码器初始化（I2C 引脚配置、I2C 初始化、寄存器写入）
- 演示注册 IMS 语音事件回调（来电 RING、挂断、通话连接）
- 演示等待 IMS PDP 获取 IP 后进入可通话状态
- 演示收到来电后通过信号量通知主任务，并执行自动应答（ATA）
- 演示通话保持指定时长后挂断（ATH）
- 演示主动拨号、保持通话并再次挂断的完整流程

## 快速上手

### 1. 安装 UniRTOS 工具链

- [开发准备](https://www.quectel.com.cn/unirtos/docs?docs_page=快速上手/开发准备/开发准备.html)
- [安装交叉编译工具链](https://www.quectel.com.cn/unirtos/docs?docs_page=快速上手/环境搭建/环境搭建.html)
- [安装 Python3](https://www.python.org/downloads/)
- [安装 git](https://git-scm.com)
- 安装 unirtos-cli：`pip install unirtos-cli`

以上工具安装完成后，确认以下命令可用：

```bash
python --version # Python3
git --version
unirtos --version # 1.0.5 及以上版本
unirtos-cli version # 1.0.11 及以上版本
```

### 2. 使用 unirtos-cli 拉取 demo

先查看可用 demo 与版本：

```bash
unirtos-cli ls-demos
```

创建本 demo 工程：

```bash
unirtos-cli new -r unirtos-voice-call-demos
```

如需指定版本：

```bash
unirtos-cli new -r unirtos-voice-call-demos -v 1.0.0
```

### 3. 进入工程并编译

```bash
cd unirtos-voice-call-demos-1.0.0
unirtos-cli env-setup
unirtos-cli build
```

## 常用命令

```bash
# 打开 SDK 菜单配置
unirtos-cli menuconfig

# 清理构建产物
unirtos-cli clean
```

## 技术社区

技术社区：https://forumschinese.quectel.com/c/66-category/66

## 贡献指南

欢迎参与共建，建议按以下方式提交：
- 提交前先执行一次基础验证：env-setup、build、clean。
- 使用清晰的提交说明，描述改动目的、影响范围和验证结果。
- 新增功能或行为变化时，同步更新 README 与相关文档。
- 通过 Issue 或 Pull Request 提交问题修复与功能改进。
