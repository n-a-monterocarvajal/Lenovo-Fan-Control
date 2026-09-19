#ifndef UI_STRINGS_H
#define UI_STRINGS_H
#define VERSION "v0.7.1"
/* The original manual controls and these new controls share the Windows locale. */
enum UiText {
    UI_SAVE_ERROR, UI_TEMPERATURE_PAIR, UI_CPU_ONLY, UI_UNAVAILABLE_AUTO,
    UI_UNAVAILABLE_MANUAL, UI_AUTO, UI_MANUAL, UI_TIMER_ERROR, UI_THRESHOLD_ERROR,
    UI_THRESHOLD_ERROR_TITLE, UI_MENU_AUTO, UI_MENU_SETTINGS, UI_LOADING,
    UI_REGISTER_ERROR, UI_WINDOW_ERROR, UI_ERROR, UI_SETTINGS_TITLE,
    UI_SETTINGS_HINT, UI_SETTINGS_HIGH, UI_SETTINGS_NORMAL, UI_SETTINGS_BAND,
    UI_SAVE, UI_CANCEL, UI_MENU_STARTUP, UI_ELEVATE_PROMPT, UI_APP_NAME, UI_NOTE,
    UI_ALREADY_RUNNING, UI_DRIVER_ERROR, UI_AT_LOW, UI_AT_HIGH, UI_AT_NORMAL,
    UI_MENU_LOW, UI_MENU_HIGH, UI_MENU_NORMAL, UI_MENU_ABOUT, UI_MENU_EXIT, UI_ABOUT_TEXT,
    UI_MENU_ELEVATED
};
static int ui_language; /* 0 = English, 1 = Spanish, 2 = Chinese */
static const wchar_t *const ui_strings[][3] = {
    { L"Could not save temperature settings.", L"No se pudieron guardar los ajustes de temperatura.", L"无法保存温度设置。" },
    { L"CPU: %.0f °C / GPU: %.0f °C", L"CPU: %.0f °C / GPU: %.0f °C", L"CPU: %.0f °C / GPU: %.0f °C" },
    { L"CPU: %.0f °C / GPU: unavailable", L"CPU: %.0f °C / GPU: no disponible", L"CPU: %.0f °C / GPU: 不可用" },
    { L"Temperature unavailable; High Speed for safety", L"Temperatura no disponible: se usa velocidad alta por precaución", L"温度不可用；保持高转速" },
    { L"Temperature unavailable (check sensors)", L"Temperatura no disponible (revisa los sensores)", L"温度不可用（请检查传感器）" },
    { L"automatic", L"automático", L"自动" },
    { L"manual", L"manual", L"手动" },
    { L"Could not start temperature polling. Automatic mode is unavailable.", L"No se pudo iniciar la lectura de temperatura. El modo automático no está disponible.", L"无法启动温度监控。自动模式不可用。" },
    { L"Thresholds must satisfy: 20 ≤ Normal < High ≤ 100 °C.", L"Los umbrales deben cumplir: 20 ≤ Normal < Alta ≤ 100 °C.", L"温度阈值必须满足：20 ≤ 正常 < 高速 ≤ 100 °C。" },
    { L"Invalid thresholds", L"Umbrales no válidos", L"温度阈值无效" },
    { L"Automatic (temperature)", L"Automático por temperatura", L"自动（按温度）" },
    { L"Temperature thresholds...", L"Umbrales de temperatura...", L"温度阈值..." },
    { L"Reading temperatures...", L"Leyendo temperaturas...", L"正在读取温度..." },
    { L"Window registration failed.", L"No se pudo registrar la ventana.", L"窗口注册失败。" },
    { L"Window creation failed.", L"No se pudo crear la ventana.", L"窗口创建失败。" },
    { L"Error", L"Error", L"错误" },
    { L"Temperature thresholds", L"Umbrales de temperatura", L"温度阈值" },
    { L"The highest CPU/GPU temperature controls the fan.", L"Se usa la temperatura más alta entre CPU y GPU.", L"根据 CPU 和 GPU 的最高温度控制风扇。" },
    { L"High Speed at or above (°C):", L"Velocidad alta a partir de (°C):", L"高转速启动温度 (°C)：" },
    { L"Normal Speed at or below (°C):", L"Velocidad normal al bajar a (°C):", L"恢复正常转速温度 (°C)：" },
    { L"Between the two values, the fan keeps its last mode instead of switching back and forth.",
      L"Entre ambos valores, el ventilador no cambia de modo: así se evita que suba y baje de golpe.",
      L"在两个阈值之间风扇保持原有模式，避免频繁切换。" },
    { L"Save", L"Guardar", L"保存" },
    { L"Cancel", L"Cancelar", L"取消" },
    { L"Start with Windows", L"Iniciar con Windows", L"开机启动" },
    { L"Reading CPU temperature needs administrator privileges. Restart elevated now?\n\n"
      L"If you say no, Automatic mode will request High Speed for safety, since CPU "
      L"temperature stays unavailable.",
      L"Para leer la temperatura de CPU hace falta reiniciar el programa como administrador. ¿Reiniciar ahora?\n\n"
      L"Si eliges que no, el programa sigue abierto sin permisos de administrador, y el modo "
      L"Automático va a mantener la velocidad alta de forma predeterminada.",
      L"读取 CPU 温度需要管理员权限。是否现在以管理员身份重启？\n\n"
      L"如果选择否，由于 CPU 温度仍不可用，自动模式将为安全起见请求高转速。" },
    { L"Lenovo Fan Control", L"Lenovo Fan Control", L"联想风扇控制" },
    { L"Note", L"Aviso", L"提示" },
    { L"The program is running.", L"El programa ya está en ejecución.", L"程序已经在运行中。" },
    { L"Failed to open \\\\.\\EnergyDrv. Unsupported device or something wrong with Lenovo ACPI-Compliant Virtual Power Controller driver.",
      L"No se pudo acceder a \\\\.\\EnergyDrv. Comprueba que el equipo sea compatible y que esté instalado el controlador Lenovo ACPI-Compliant Virtual Power Controller.",
      L"无法访问\\\\.\\EnergyDrv。本设备不支持或Lenovo ACPI-Compliant Virtual Power Controller驱动异常。" },
    { L"Low Speed", L"Velocidad baja", L"低转速" },
    { L"High Speed", L"Velocidad alta", L"高转速" },
    { L"Normal Speed", L"Velocidad normal", L"正常转速" },
    { L"Low Speed\tCtrl+Alt+F10", L"Velocidad baja\tCtrl+Alt+F10", L"低转速\tCtrl+Alt+F10" },
    { L"High Speed\tCtrl+Alt+F11", L"Velocidad alta\tCtrl+Alt+F11", L"高转速\tCtrl+Alt+F11" },
    { L"Normal Speed\tCtrl+Alt+F12", L"Velocidad normal\tCtrl+Alt+F12", L"正常转速\tCtrl+Alt+F12" },
    { L"About", L"Acerca de", L"关于" },
    { L"Exit", L"Salir", L"退出" },
    { L"Lenovo Fan Control " VERSION L"\n\n"
      L"Control fan for Lenovo laptops with Lenovo ACPI-Compliant Virtual Power Controller driver on Windows.\n\n"
      L"Original: jiarandiana0307 (Kira Diana)\nhttps://github.com/jiarandiana0307/Lenovo-Fan-Control\n\n"
      L"Fork and temperature control: n-a-monterocarvajal\nhttps://github.com/n-a-monterocarvajal/Lenovo-Fan-Control\n\n"
      L"Sensors: LibreHardwareMonitor (MPL-2.0). Inspired by IdeaFan, by Andrius allstone Stasauskas.\n\n"
      L"Disclaimer: This program is not responsible for possible damage of any kind, use it at your own risk.",
      L"Lenovo Fan Control " VERSION L"\n\n"
      L"Control del ventilador para portátiles Lenovo con el controlador Lenovo ACPI-Compliant Virtual Power Controller en Windows.\n\n"
      L"Proyecto original: jiarandiana0307 (Kira Diana)\n"
      L"https://github.com/jiarandiana0307/Lenovo-Fan-Control\n\n"
      L"Fork y control por temperatura: n-a-monterocarvajal\n"
      L"https://github.com/n-a-monterocarvajal/Lenovo-Fan-Control\n\n"
      L"Lectura de sensores: LibreHardwareMonitor (MPL-2.0).\n"
      L"Función inspirada en IdeaFan, de Andrius allstone Stašauskas.\n\n"
      L"Uso bajo tu responsabilidad. El proyecto no se hace responsable de posibles daños.",
      L"联想风扇控制 " VERSION L"\n\n"
      L"在Windows上通过Lenovo ACPI-Compliant Virtual Power Controller驱动控制联想笔记本电脑的风扇。\n\n"
      L"本程序已开源：https://github.com/jiarandiana0307/Lenovo-Fan-Control\n\n"
      L"Fork: n-a-monterocarvajal\nhttps://github.com/n-a-monterocarvajal/Lenovo-Fan-Control\n\n"
      L"免责声明：本程序不对任何可能的损坏负责，风险自担。" },
    { L"Always run as administrator", L"Iniciar siempre como administrador", L"始终以管理员身份运行" }
};
static const wchar_t *ui(enum UiText text) { return ui_strings[text][ui_language]; }
#endif
