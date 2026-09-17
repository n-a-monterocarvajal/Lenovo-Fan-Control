# Lenovo Laptop Fan Control

Language: [Español](README.md) | [中文](README.zh_CN.md)

---

Control fan for Lenovo laptops with `Lenovo ACPI-Compliant Virtual Power Controller` driver on Windows.

This project is for the Lenovo laptops whose fan failed to be controlled with mainstream fan control applications (e.g. Notebook FanControl, SpeedFan, Fan Control) on Windows.

However, this project is not a perfect solution for fan control. It can only control the fan to spin at the minimum speed, maximum speed and the normal speed, and dosen't provide any accurate speed control.

This fork adds optional temperature thresholds to the original tray application.
It switches between High Speed and the laptop's Normal Speed automatically,
without installing the FanControl or Open Hardware Monitor applications.
For full fan curves, [FanControl](https://github.com/Rem0o/FanControl.Releases)
and [FanControl.LenovoPlugin](https://github.com/jiarandiana0307/FanControl.LenovoPlugin)
remain alternatives.

## Automatic temperature control

1. `LenovoFanControl-x64.exe` (or `-x86.exe`) is the only executable; the
   LibreHardwareMonitor-based sensor helper is embedded in it, not a separate file.
2. Manual speed control and the Lenovo EnergyDrv driver don't need
   Administrator, so the app opens without a UAC prompt. Reading CPU
   temperature does need it: turning on **Automatic (temperature)** while
   unelevated offers to restart the app elevated, and on first elevated run
   it silently installs the [PawnIO sensor driver](https://pawnio.eu/) if
   missing (its signed installer ships embedded too). Declining keeps the app
   unelevated, but Automatic then requests High Speed for safety, since CPU
   temperature stays unavailable. The embedded sensor helper needs .NET
   Framework 4.7.2 or later, already present on Windows 10/11.
3. Open **Temperature thresholds...** in the tray menu. Defaults are **70 C**
   to enter High Speed and **65 C** to return to Normal Speed.
4. Check **Automatic (temperature)**, or start with `--auto`.

The menu shows current CPU and GPU readings. CPU Package (or AMD die temperature)
is preferred; otherwise the highest available absolute CPU temperature is used.
The primary GPU Core temperature is used for each detected GPU. The hottest
CPU/GPU value controls the fan: either can trigger High Speed; all monitored
readings must be at or below the lower threshold to return to Normal Speed.
GPU hotspot, VRAM, disks, motherboard sensors and Distance to TjMax are excluded.
A GPU without a supported core sensor is shown as unavailable; CPU monitoring
is required. If a previously readable GPU stops reporting, that is a sensor failure.

The speed and temperatures are the two informational rows at the top of the tray
menu. Temperatures remain visible in manual mode. The UI is available in
English, neutral Spanish, and Chinese, chosen automatically from the Windows
language. About credits both the original author and this fork.

Between the two thresholds the previous speed is retained (hysteresis).

**Start with Windows** in the tray menu adds/removes a per-user Run registry
entry (no installer, no scheduled task). It launches unelevated, same as a
normal double-click; Automatic mode still prompts for elevation the first
time it needs to read the CPU.

Settings accept `20 <= Normal < High <= 100` degrees Celsius. Defaults are starting
points, not hardware-specific maximum temperatures. Readings arrive every two
seconds. Missing, invalid or stale readings (10 seconds without an update) request
High Speed and show **Temperature unavailable**. Startup also requests High Speed
until a fresh reading arrives. After resolving a sensor/driver problem, toggle
Automatic off and on to restart the sensor helper.

Choosing any manual speed or its hotkey disables Automatic but keeps temperature
monitoring active. Unchecking Automatic returns to Normal Speed. Settings
persist in `%LOCALAPPDATA%\LenovoFanControl\settings.ini`; manual
command-line speed flags override saved Automatic for that launch. Manual
Low Speed has no thermal override.

Unlike IdeaFan's broader set of OHM temperatures, this mode deliberately monitors
CPU/GPU cooling. It preserves the Lenovo dust-removal method and its limitations;
High Speed is a requested mode, not a measurement of fan RPM.

## Build and test this fork

Install Visual Studio 2022 Build Tools with **Desktop development with C++**, a
Windows SDK, and a .NET SDK. From PowerShell:

```powershell
./build.ps1 -Architecture x64 -Test
./build.ps1 -Architecture x86 -Test
```

`bin/x64/LenovoFanControl-x64.exe` and `bin/x86/LenovoFanControl-x86.exe` are
the distributables; the sensor helper and its dependencies are merged into
them, nothing else needs to ship alongside. Tests simulate temperature traces and
driver calls, and exercise the English/Spanish tray and settings dialog;
they do not operate the real fan. Hardware compatibility must still
be checked on the target Lenovo laptop. The original MinGW Makefile builds the
native application only; `build.ps1` also builds/packages the sensor helper.
See [third-party dependencies](THIRD-PARTY-NOTICES.md).

# Prerequisites

- Lenovo laptop
- Windows operating system
- `Lenovo ACPI-Compliant Virtual Power Controller` driver installed

# Usage

1. Build this fork as described above. The [upstream releases](https://github.com/jiarandiana0307/Lenovo-Fan-Control/releases) do not contain the automatic temperature option.

2. Double-click the LenovoFanControl program to run it, then you will see it in system tray.

If a message box saying `Failed to open\\.\EnergyDrv` popped up, it means the Lenovo driver not found or dosen't work as expected. Otherwise, the program works fine and the fan will spin at maximum speed.

![Menu Screenshot](github/menu-screenshot.jpg)

Click the program icon in the system tray, a menu will show up. The first line of the menu tell you the state of the fan, which could be one of the three: 

1. `Low Speed`: the fan is spinning at minimum speed.
2. `High Speed`: the fan is spinning at maximum speed.
3. `Normal Speed`: the fan is spinning at normal speed.

You can click the `Low Speed` and `High Speed` item on the menu, or use the corresponding hotkeys `Ctrl+Alt+F10` and `Ctrl+Alt+F11` to keep the fan spinning at minimun and maximum speed respectively. Also, you can click the `Normal Speed` item on the menu or use the corresponding hotkey `Ctrl+Alt+F12` to return the fan to its normal speed.

Finally, you can click the `Exit` item on the menu to terminate the program, then the fan will spin at normal speed.

To select the start speed of the fan, you can run the program with command line parameter `--low-speed`, `--normal-speed` and `--high-speed`, which will set the fan to low speed, normal speed and high speed at start respectively. The default behavior is to set the fan to high speed if non of these parameters are given. For example, if you want to keep the fan spinning at low speed at start, you can run the command: `LenovoFanControl-x64.exe --low-speed`

**Note:** Manual `Low Speed` disables automatic temperature control and can lead to high hardware temperatures. Automatic mode never selects Low Speed.

# Theory

Normally, fan in laptop is controlled by Embedded Controller (EC) which is a device responsible for feeding other parts of the system the electric voltage they need. Therefore if EC pass more voltage to the cooling system, this cause fan to spins at maximum speed. For controlling the EC we can change the EC registers to achieve functionality we want, but for some models, we can't find the registers which was dedicated to controlling the fan speed from datasheets. So one reverse engineering the Lenovo Energy Manager software to find how Dust Removal feature of this app actually works and build this program with it. Then it turns out that this software communicate to the EC through `Lenovo ACPI-Compliant Virtual Power Controller` kernel driver. And it's exactly how this project works.

If you have `Lenovo ACPI-Compliant Virtual Power Controller` driver installed, there will be a `\\.\EnergyDrv` device on your system. This device is created by the Lenovo driver to expose the interfaces for communication with other applications, e.g. Lenovo Energy Manager. The driver provides a dust removal function that can control the fan. Therefore, with win32 API, it's easy to read and write specific bytes of the device to control the Lenovo driver, then the driver controls EC, and the EC controls the fan. See the diagram below.

![Diagram](github/diagram.jpg)

But there is a problem with this approach, the fan spins periodically. After we instruct the driver to carry out dust removal, the fan spins at maximum speed for about 9 seconds then stops for 2 seconds, and then the next cycle until 2min later. The dust removal is controlled automatically by EC itself, and sometimes may suddenly stop during the 9 seconds of spinning which leads to fan stop spinning for something for 1 to 9 seconds.

For workaround about this problem, firstly, we ask the driver to carry out dust removal, wait for 9 seconds. Then we ask the driver to stop the procedure manually to reset the timing. Finally, we ask the driver to start over the procedure immediately before the fan stop spinning. And then wait for another 9 seconds, then stop, then the next cycle and so on. With the fast on and off switching, the fan won't stop and will spin at the maximum speed all the time except that the speed of the fan will slow down a little bit for a short period of time during the switching time.

# Disclaimer

This project is not responsible for possible damage of any kind, use it at your own risk.

# References

- [IdeaFan][IdeaFan]
- [FanControl][FanControl]
- [Lenovo-IdeaPad-Z500-Fan-Controller][Lenovo-IdeaPad-Z500-Fan-Controller]
- [Windows Drivers Reverse Engineering Methodology][windows-drivers-reverse-engineering-methodology]

[IdeaFan]: https://www.allstone.lt/ideafan/
[FanControl]: https://github.com/bitrate16/FanControl
[Lenovo-IdeaPad-Z500-Fan-Controller]: https://github.com/Soberia/Lenovo-IdeaPad-Z500-Fan-Controller
[windows-drivers-reverse-engineering-methodology]: https://voidsec.com/windows-drivers-reverse-engineering-methodology/
