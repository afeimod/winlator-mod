# Winlator-Glibc

Winlator-Glibc 是官方 Winlator 的一个分支。Glibc 版本提供了额外的性能和稳定性改进。目标是提供一个更友好的社区替代方案，欢迎协作。

该分支也代表了 WinlatorXR 版本，用于在虚拟头戴设备中运行应用。

# Modify by
- Waim908 [https://github.com/Waim908]
- Afeimod [https://github.com/afeimod]
- Hostei [https://github.com/hostei33]
- Ewt45[https://github.com/ewt45]
- Moze [https://github.com/moze30]


# 设备要求
* Android 8 或更新版本，且配备 ARM64 CPU
* 兼容的 GPU（Adreno GPU 支持最佳）
* 支持传统存储（据报告，Coloros 15 和 Oxygenos 15 不支持）

# 编译

1. 在 Android Studio 中打开项目（我们以最新的稳定版本为目标）

2. 安装 Android Studio 提示所需的依赖项

3. 通过 USB 连接手机并启用 USB 调试

4. 点击运行（绿色播放图标）

# 链接
- [最新Rootfs下载](https://github.com/moze30/rootfs-winlator-glibc)
- [为Winlator-glibc维护的mesa仓库](https://github.com/moze30/mesa-for-android-container)
- [原项目下载](https://github.com/longjunyu2/winlator/releases)
- [SideQuest 上的 WinlatorXR](https://sidequestvr.com/app/37320/winlatorxr)

# 致谢与第三方应用
- rootfs-winlator-glibc（[github.com/moze30/rootfs-winlator-glibc](https://github.com/moze30/rootfs-winlator-glibc)）
- Termux-pacman（[github.com/termux-pacman/glibc-packages](https://github.com/termux-pacman/glibc-packages)）
- mesa-for-android-container（[github.com/lfdevs/mesa-for-android-container](https://github.com/lfdevs/mesa-for-android-container)）

---

<p align="center">
	<img src="logo.png" width="376" height="128" alt="Winlator 徽标" />  
</p>

# Winlator

Winlator 是一款 Android 应用程序，可让您借助 Wine 和 Box64 运行 Windows（x86_64）应用程序。

# 安装

1. 从 [GitHub Releases](https://github.com/brunodev85/winlator/releases) 下载并安装 APK（Winlator_9.0.apk）
2. 启动应用，等待安装过程完成

----

[![在 YouTube 上播放](https://img.youtube.com/vi/ETYDgKz4jBQ/3.jpg)](https://www.youtube.com/watch?v=ETYDgKz4jBQ)
[![在 YouTube 上播放](https://img.youtube.com/vi/9E4wnKf2OsI/2.jpg)](https://www.youtube.com/watch?v=9E4wnKf2OsI)
[![在 YouTube 上播放](https://img.youtube.com/vi/czEn4uT3Ja8/2.jpg)](https://www.youtube.com/watch?v=czEn4uT3Ja8)
[![在 YouTube 上播放](https://img.youtube.com/vi/eD36nxfT_Z0/2.jpg)](https://www.youtube.com/watch?v=eD36nxfT_Z0)

----

# 实用提示

- 如果遇到性能问题，请尝试在容器设置 -> 高级选项卡中将 Box64 预设更改为 `Performance`（性能）。
- 对于使用 .NET Framework 的应用程序，请尝试在开始菜单 -> 系统工具中安装 `Wine Mono`。
- 如果某些旧游戏无法打开，请尝试在容器设置 -> 环境变量中添加环境变量 `MESA_EXTENSION_MAX_YEAR=2003`。
- 尝试使用 Winlator 主屏幕上的快捷方式运行游戏，您可以在其中为每个 game 定义单独的设置。
- 为了正确显示低分辨率游戏，请尝试在快捷方式设置中启用 `Force Fullscreen`（强制全屏）选项。
- 为了提高使用 Unity 引擎的游戏的稳定性，请尝试将 Box64 预设更改为 `Stability`（稳定），或者在快捷方式设置中添加执行参数 `-force-gfx-direct`。

# 信息

该项目自 1.0 版本以来一直在持续开发，当前应用源代码最高版本为 7.1。我不经常更新此仓库，正是为了避免在 Winlator 正式发布之前出现非官方版本。

# 致谢与第三方应用
- Ubuntu 根文件系统（[Focal Fossa](https://releases.ubuntu.com/focal)）
- Wine（[winehq.org](https://www.winehq.org/)）
- Box64 作者：[ptitseb](https://github.com/ptitSeb)
- PRoot（[proot-me.github.io](https://proot-me.github.io)）
- Mesa（Turnip/Zink/VirGL）（[mesa3d.org](https://www.mesa3d.org)）
- DXVK（[github.com/doitsujin/dxvk](https://github.com/doitsujin/dxvk)）
- VKD3D（[gitlab.winehq.org/wine/vkd3d](https://gitlab.winehq.org/wine/vkd3d)）
- D8VK（[github.com/AlpyneDreams/d8vk](https://github.com/AlpyneDreams/d8vk)）
- CNC DDraw（[github.com/FunkyFr3sh/cnc-ddraw](https://github.com/FunkyFr3sh/cnc-ddraw)）
- Hangover ([github.com/AndreRH/hangover](https://github.com/AndreRH/hangover))
- FEX ([github.com/FEX-Emu/FEX](https://github.com/FEX-Emu/FEX))

非常感谢 [ptitSeb](https://github.com/ptitSeb)（Box64）、[Danylo](https://blogs.igalia.com/dpiliaiev/tags/mesa/)（Turnip）、[alexvorxx](https://github.com/alexvorxx)（Mods/提示）以及其他贡献者。<br>
感谢所有相信这个项目的人。
