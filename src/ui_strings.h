#ifndef UI_STRINGS_H
#define UI_STRINGS_H
/* The original manual controls and these new controls share the Windows locale. */
enum UiText {
    UI_SAVE_ERROR, UI_TEMPERATURE_PAIR, UI_CPU_ONLY, UI_UNAVAILABLE_AUTO,
    UI_UNAVAILABLE_MANUAL, UI_AUTO, UI_MANUAL, UI_TIMER_ERROR, UI_THRESHOLD_ERROR,
    UI_THRESHOLD_ERROR_TITLE, UI_MENU_AUTO, UI_MENU_SETTINGS, UI_LOADING,
    UI_REGISTER_ERROR, UI_WINDOW_ERROR, UI_ERROR, UI_SETTINGS_TITLE,
    UI_SETTINGS_HINT, UI_SETTINGS_HIGH, UI_SETTINGS_NORMAL, UI_SETTINGS_BAND,
    UI_SAVE, UI_CANCEL
};
static int ui_language; /* 0 = English, 1 = Spanish, 2 = Chinese */
static const wchar_t *const ui_strings[][3] = {
    { L"Could not save temperature settings.", L"No se pudieron guardar los ajustes de temperatura.", L"无法保存温度设置。" },
    { L"CPU: %.1f °C / GPU: %.1f °C", L"CPU: %.1f °C / GPU: %.1f °C", L"CPU: %.1f °C / GPU: %.1f °C" },
    { L"CPU: %.1f °C / GPU: unavailable", L"CPU: %.1f °C / GPU: no disponible", L"CPU: %.1f °C / GPU: 不可用" },
    { L"Temperature unavailable; High Speed for safety", L"Temperatura no disponible; velocidad alta por seguridad", L"温度不可用；保持高转速" },
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
    { L"Cancel", L"Cancelar", L"取消" }
};
static const wchar_t *ui(enum UiText text) { return ui_strings[text][ui_language]; }
#endif
