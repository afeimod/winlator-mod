<p align="center">
	<img src="logo.png" width="376" height="128" alt="Winlator Logo" />  
</p>

# 介绍

Winlator 是一个 Android 应用程序，它允许您通过 Wine 和 Box86/Box64 运行 Windows（x86_64）应用程序。

# 安装

1. 从 [GitHub 发布页面](https://github.com/brunodev85/winlator/releases) 下载并安装 APK（Winlator_7.1.apk）
2. 启动应用程序并等待安装过程完成

----

[![Play on Youtube](https://img.youtube.com/vi/8PKhmT7B3Xo/1.jpg)](https://www.youtube.com/watch?v=8PKhmT7B3Xo)
[![Play on Youtube](https://img.youtube.com/vi/9E4wnKf2OsI/2.jpg)](https://www.youtube.com/watch?v=9E4wnKf2OsI)
[![Play on Youtube](https://img.youtube.com/vi/czEn4uT3Ja8/2.jpg)](https://www.youtube.com/watch?v=czEn4uT3Ja8)
[![Play on Youtube](https://img.youtube.com/vi/eD36nxfT_Z0/2.jpg)](https://www.youtube.com/watch?v=eD36nxfT_Z0)

----

# 使用建议

- 如果你遇到性能问题，请尝试在“容器设置”->“高级”选项卡中更改 Box86/Box64 的预设。
- 对于使用 .NET Framework 的应用程序，请尝试在开始菜单 -> 系统工具中找到并安装 Wine Mono。
- 如果一些较旧的游戏无法打开，请尝试在“容器设置”->“环境变量”中添加环境变量 MESA_EXTENSION_MAX_YEAR=2003。
- 尝试使用 Winlator 主屏幕上的快捷方式运行游戏，您可以在那里为每个游戏定义单独的设置。
- 为了加快安装程序的速度，请尝试在“容器设置”->“高级”选项卡中将 Box86/Box64 预设更改为“中等”。

# 鸣谢和第三方应用程序

- Ubuntu RootFs ([Focal Fossa](https://releases.ubuntu.com/focal))
- Wine ([winehq.org](https://www.winehq.org/))
- Box86/Box64 by [ptitseb](https://github.com/ptitSeb)
- PRoot ([proot-me.github.io](https://proot-me.github.io))
- Mesa (Turnip/Zink/VirGL) ([mesa3d.org](https://www.mesa3d.org))
- DXVK ([github.com/doitsujin/dxvk](https://github.com/doitsujin/dxvk))
- VKD3D ([gitlab.winehq.org/wine/vkd3d](https://gitlab.winehq.org/wine/vkd3d))
- D8VK ([github.com/AlpyneDreams/d8vk](https://github.com/AlpyneDreams/d8vk))
- CNC DDraw ([github.com/FunkyFr3sh/cnc-ddraw](https://github.com/FunkyFr3sh/cnc-ddraw))

感谢 [ptitSeb](https://github.com/ptitSeb) (Box86/Box64), [Danylo](https://blogs.igalia.com/dpiliaiev/tags/mesa/) (Turnip), [alexvorxx](https://github.com/alexvorxx) (Mods/Tips) 和其他贡献者。

感谢所有相信这个项目的人。
