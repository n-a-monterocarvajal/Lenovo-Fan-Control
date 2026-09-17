#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <wchar.h>

#include "fanctrl.h"
#include "fan_worker.h"
#include "auto_control.h"
#include "temperature.h"
#include "ui_strings.h"
#include "../res/resource.h"

#define WM_TRAYICON (WM_USER + 1)

#define VERSION "v0.6"

enum TrayMenuIDs {
    ID_TRAY_APP_ICON = 1001,
    ID_TRAY_STATE,
    ID_TRAY_LOW_SPEED,
    ID_TRAY_HIGH_SPEED,
    ID_TRAY_NORMAL_SPEED,
    ID_TRAY_ABOUT,
    ID_TRAY_EXIT,
    ID_TRAY_AUTO,
    ID_TRAY_TEMPERATURE,
    ID_TRAY_SETTINGS,
    ID_TRAY_STARTUP,
};

enum HotKeyIDs {
    HOTKEY_LOW_SPEED,
    HOTKEY_HIGH_SPEED,
    HOTKEY_NORMAL_SPEED,
};

typedef struct {
    LPCWSTR app_name;
    LPCWSTR note;
    LPCWSTR program_is_running;
    LPCWSTR failed_to_open_driver;
    LPCWSTR state;
    LPCWSTR menu_at_low_speed;
    LPCWSTR menu_at_high_speed;
    LPCWSTR menu_at_normal_speed;
    LPCWSTR menu_low_speed;
    LPCWSTR menu_high_speed;
    LPCWSTR menu_normal_speed;
    LPCWSTR menu_about;
    LPCWSTR menu_exit;
    LPCWSTR about_text;
} LangResources;

const LangResources en_US = {
    TEXT("Lenovo Fan Control"),
    TEXT("Note"),
    TEXT("The program is running."),
    TEXT("Failed to open \\\\.\\EnergyDrv. Unsupported device or something wrong with Lenovo ACPI-Compliant Virtual Power Controller driver."),
    TEXT("State"),
    TEXT("Low Speed"),
    TEXT("High Speed"),
    TEXT("Normal Speed"),
    TEXT("Low Speed\tCtrl+Alt+F10"),
    TEXT("High Speed\tCtrl+Alt+F11"),
    TEXT("Normal Speed\tCtrl+Alt+F12"),
    TEXT("About"),
    TEXT("Exit"),
    TEXT("Lenovo Fan Control " VERSION "\n\n\
Control fan for Lenovo laptops with Lenovo ACPI-Compliant Virtual Power Controller driver on Windows.\n\n\
Original: jiarandiana0307 (Kira Diana)\nhttps://github.com/jiarandiana0307/Lenovo-Fan-Control\n\n\
Fork and temperature control: n-a-monterocarvajal\nhttps://github.com/n-a-monterocarvajal/Lenovo-Fan-Control\n\n\
Sensors: LibreHardwareMonitor (MPL-2.0). Inspired by IdeaFan, by Andrius allstone Stasauskas.\n\n\
Disclaimer: This program is not responsible for possible damage of any kind, use it at your own risk.")
};

const LangResources zh_CN = {
    TEXT("联想风扇控制"),
    TEXT("提示"),
    TEXT("程序已经在运行中。"),
    TEXT("无法访问\\\\.\\EnergyDrv。本设备不支持或Lenovo ACPI-Compliant Virtual Power Controller驱动异常。"),
    TEXT("状态"),
    TEXT("低转速"),
    TEXT("高转速"),
    TEXT("正常转速"),
    TEXT("低转速\tCtrl+Alt+F10"),
    TEXT("高转速\tCtrl+Alt+F11"),
    TEXT("正常转速\tCtrl+Alt+F12"),
    TEXT("关于"),
    TEXT("退出"),
    TEXT("联想风扇控制 " VERSION "\n\n\
在Windows上通过Lenovo ACPI-Compliant Virtual Power Controller驱动控制联想笔记本电脑的风扇。\n\n\
本程序已开源：https://github.com/jiarandiana0307/Lenovo-Fan-Control\n\n\
Fork: n-a-monterocarvajal\nhttps://github.com/n-a-monterocarvajal/Lenovo-Fan-Control\n\n\
免责声明：本程序不对任何可能的损坏负责，风险自担。")
};

const LangResources es = {
    L"Lenovo Fan Control",
    L"Aviso",
    L"El programa ya está en ejecución.",
    L"No se pudo acceder a \\\\.\\EnergyDrv. Comprueba que el equipo sea compatible y que esté instalado el controlador Lenovo ACPI-Compliant Virtual Power Controller.",
    L"Estado",
    L"Velocidad baja",
    L"Velocidad alta",
    L"Velocidad normal",
    L"Velocidad baja\tCtrl+Alt+F10",
    L"Velocidad alta\tCtrl+Alt+F11",
    L"Velocidad normal\tCtrl+Alt+F12",
    L"Acerca de",
    L"Salir",
    L"Lenovo Fan Control " VERSION L"\n\n"
    L"Control del ventilador para portátiles Lenovo con el controlador Lenovo ACPI-Compliant Virtual Power Controller en Windows.\n\n"
    L"Proyecto original: jiarandiana0307 (Kira Diana)\n"
    L"https://github.com/jiarandiana0307/Lenovo-Fan-Control\n\n"
    L"Fork y control por temperatura: n-a-monterocarvajal\n"
    L"https://github.com/n-a-monterocarvajal/Lenovo-Fan-Control\n\n"
    L"Lectura de sensores: LibreHardwareMonitor (MPL-2.0).\n"
    L"Función inspirada en IdeaFan, de Andrius allstone Stašauskas.\n\n"
    L"Uso bajo tu responsabilidad. El proyecto no se hace responsable de posibles daños."
};

const LangResources* lang = &en_US;
NOTIFYICONDATA nid;
HMENU hMenu;
enum FanSpeed fan_speed_set_at_start = HIGH_SPEED;
static enum FanSpeed current_speed = HIGH_SPEED;
static int automatic, high_threshold = 70, normal_threshold = 65, auto_high = 1;
static WCHAR settings_path[MAX_PATH];

#define STARTUP_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define STARTUP_VALUE L"LenovoFanControl"

/* Quoted so a path with spaces still runs correctly from the Run key. */
static int get_quoted_module_path(WCHAR *path, DWORD capacity) {
    DWORD length = GetModuleFileNameW(NULL, path + 1, capacity - 2);
    if (!length || length >= capacity - 2) return 0;
    path[0] = L'"';
    path[length + 1] = L'"';
    path[length + 2] = 0;
    return 1;
}

/* Elevation is still required (EnergyDrv), so Windows will still prompt for UAC
   at login when this is on; there is no way around that without Task Scheduler. */
static int is_startup_enabled(void) {
    WCHAR expected[MAX_PATH + 2], current[MAX_PATH + 2];
    HKEY key;
    DWORD size = sizeof(current), type;
    LONG result;
    if (!get_quoted_module_path(expected, MAX_PATH + 2)) return 0;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, STARTUP_KEY, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) return 0;
    result = RegQueryValueExW(key, STARTUP_VALUE, NULL, &type, (BYTE *)current, &size);
    RegCloseKey(key);
    return result == ERROR_SUCCESS && type == REG_SZ && !wcscmp(expected, current);
}

static void set_startup_enabled(int enabled) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, STARTUP_KEY, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS) return;
    if (enabled) {
        WCHAR path[MAX_PATH + 2];
        if (get_quoted_module_path(path, MAX_PATH + 2))
            RegSetValueExW(key, STARTUP_VALUE, 0, REG_SZ, (const BYTE *)path, (DWORD)(wcslen(path) + 1) * sizeof(WCHAR));
    } else {
        RegDeleteValueW(key, STARTUP_VALUE);
    }
    RegCloseKey(key);
}

static void save_settings(void) {
    WCHAR high[16], normal[16];
    swprintf(high, 16, L"%d", high_threshold);
    swprintf(normal, 16, L"%d", normal_threshold);
    if (!settings_path[0] ||
        !WritePrivateProfileStringW(L"Temperature", L"High", high, settings_path) ||
        !WritePrivateProfileStringW(L"Temperature", L"Normal", normal, settings_path) ||
        !WritePrivateProfileStringW(L"Temperature", L"Automatic", automatic ? L"1" : L"0", settings_path))
        MessageBoxW(nid.hWnd, ui(UI_SAVE_ERROR), lang->app_name, MB_OK | MB_ICONWARNING);
}

static void load_settings(void) {
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, settings_path))) return;
    if (wcslen(settings_path) + 40 >= MAX_PATH) { settings_path[0] = 0; return; }
    wcscat(settings_path, L"\\LenovoFanControl");
    CreateDirectoryW(settings_path, NULL);
    wcscat(settings_path, L"\\settings.ini");
    high_threshold = GetPrivateProfileIntW(L"Temperature", L"High", 70, settings_path);
    normal_threshold = GetPrivateProfileIntW(L"Temperature", L"Normal", 65, settings_path);
    if (!auto_thresholds_valid(high_threshold, normal_threshold)) {
        high_threshold = 70; normal_threshold = 65;
    }
    automatic = GetPrivateProfileIntW(L"Temperature", L"Automatic", 0, settings_path) == 1;
}

void toggle_fan_low_speed() {
    current_speed = LOW_SPEED;
    ModifyMenu(hMenu, ID_TRAY_STATE, MF_STRING | MF_DISABLED, ID_TRAY_STATE, lang->menu_at_low_speed);
    fan_worker_set(LOW_SPEED);
    swprintf(nid.szTip, 128, L"%ls " VERSION L"\n%ls: %ls", lang->app_name, lang->state, lang->menu_at_low_speed);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void toggle_fan_high_speed() {
    current_speed = HIGH_SPEED;
    ModifyMenu(hMenu, ID_TRAY_STATE, MF_STRING | MF_DISABLED, ID_TRAY_STATE, lang->menu_at_high_speed);
    fan_worker_set(HIGH_SPEED);
    swprintf(nid.szTip, 128, L"%ls " VERSION L"\n%ls: %ls", lang->app_name, lang->state, lang->menu_at_high_speed);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void toggle_fan_normal_speed() {
    current_speed = NORMAL_SPEED;
    ModifyMenu(hMenu, ID_TRAY_STATE, MF_STRING | MF_DISABLED, ID_TRAY_STATE, lang->menu_at_normal_speed);
    fan_worker_set(NORMAL_SPEED);
    swprintf(nid.szTip, 128, L"%ls " VERSION L"\n%ls: %ls", lang->app_name, lang->state, lang->menu_at_normal_speed);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

static void poll_temperature(void) {
    double cpu = 0, gpu = -1;
    int valid = temperature_read(&cpu, &gpu);
    double celsius = cpu > gpu ? cpu : gpu;
    WCHAR label[160], state_label[80];
    LPCWSTR speed_label;
    if (automatic) {
        auto_high = auto_should_run_high(auto_high, celsius, valid, high_threshold, normal_threshold);
        if (auto_high) toggle_fan_high_speed(); else toggle_fan_normal_speed();
    }
    if (valid && gpu >= 0) swprintf(label, 160, ui(UI_TEMPERATURE_PAIR), cpu, gpu);
    else if (valid) swprintf(label, 160, ui(UI_CPU_ONLY), cpu);
    else wcscpy(label, ui(automatic ? UI_UNAVAILABLE_AUTO : UI_UNAVAILABLE_MANUAL));
    ModifyMenuW(hMenu, ID_TRAY_TEMPERATURE, MF_STRING | MF_DISABLED, ID_TRAY_TEMPERATURE, label);
    speed_label = current_speed == HIGH_SPEED ? lang->menu_at_high_speed :
        current_speed == LOW_SPEED ? lang->menu_at_low_speed : lang->menu_at_normal_speed;
    swprintf(state_label, 80, L"%ls (%ls)", speed_label, ui(automatic ? UI_AUTO : UI_MANUAL));
    ModifyMenuW(hMenu, ID_TRAY_STATE, MF_STRING | MF_DISABLED, ID_TRAY_STATE, state_label);
    swprintf(nid.szTip, 128, L"%.45ls\n%.80ls", state_label, label);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

static int start_temperature_monitor(void) {
    temperature_start();
    if (SetTimer(nid.hWnd, 1, 1000, NULL)) return 1;
    temperature_stop();
    MessageBoxW(nid.hWnd, ui(UI_TIMER_ERROR), lang->app_name, MB_OK | MB_ICONERROR);
    return 0;
}

static void set_automatic(int enabled, int persist) {
    automatic = enabled;
    if (enabled) {
        double cpu, gpu;
        auto_high = 1;
        toggle_fan_high_speed();
        if (!temperature_read(&cpu, &gpu) && !start_temperature_monitor()) automatic = 0;
    }
    CheckMenuItem(hMenu, ID_TRAY_AUTO, MF_BYCOMMAND | (automatic ? MF_CHECKED : MF_UNCHECKED));
    poll_temperature();
    if (persist) save_settings();
}

static INT_PTR CALLBACK SettingsProc(HWND dialog, UINT msg, WPARAM wp, LPARAM lp) {
    (void)lp;
    if (msg == WM_INITDIALOG) {
        SetWindowTextW(dialog, ui(UI_SETTINGS_TITLE));
        SetDlgItemTextW(dialog, IDC_TEMP_HINT, ui(UI_SETTINGS_HINT));
        SetDlgItemTextW(dialog, IDC_HIGH_LABEL, ui(UI_SETTINGS_HIGH));
        SetDlgItemTextW(dialog, IDC_NORMAL_LABEL, ui(UI_SETTINGS_NORMAL));
        SetDlgItemTextW(dialog, IDC_BAND_HINT, ui(UI_SETTINGS_BAND));
        SetDlgItemTextW(dialog, IDOK, ui(UI_SAVE));
        SetDlgItemTextW(dialog, IDCANCEL, ui(UI_CANCEL));
        SetDlgItemInt(dialog, IDC_HIGH_TEMP, high_threshold, FALSE);
        SetDlgItemInt(dialog, IDC_NORMAL_TEMP, normal_threshold, FALSE);
        return TRUE;
    }
    if (msg == WM_COMMAND) {
        if (LOWORD(wp) == IDOK) {
            BOOL high_ok, normal_ok;
            int high = GetDlgItemInt(dialog, IDC_HIGH_TEMP, &high_ok, FALSE);
            int normal = GetDlgItemInt(dialog, IDC_NORMAL_TEMP, &normal_ok, FALSE);
            if (!high_ok || !normal_ok || !auto_thresholds_valid(high, normal)) {
                MessageBoxW(dialog, ui(UI_THRESHOLD_ERROR), ui(UI_THRESHOLD_ERROR_TITLE), MB_OK | MB_ICONWARNING);
                return TRUE;
            }
            high_threshold = high; normal_threshold = normal;
            save_settings();
            /* New thresholds are a deliberate decision, not gradual drift: don't let
               hysteresis keep the fan on the old verdict inside the new dead band. */
            if (automatic) { auto_high = 0; poll_temperature(); }
            EndDialog(dialog, IDOK);
            return TRUE;
        }
        if (LOWORD(wp) == IDCANCEL) { EndDialog(dialog, IDCANCEL); return TRUE; }
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hwnd;
            nid.uID = ID_TRAY_APP_ICON;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.uCallbackMessage = WM_TRAYICON;
            nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
            _tcscpy(nid.szTip, lang->app_name);
            Shell_NotifyIcon(NIM_ADD, &nid);

            hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, ID_TRAY_STATE, lang->menu_at_high_speed);
            AppendMenuW(hMenu, MF_STRING | MF_DISABLED, ID_TRAY_TEMPERATURE, ui(UI_LOADING));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_LOW_SPEED, lang->menu_low_speed);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_HIGH_SPEED, lang->menu_high_speed);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_NORMAL_SPEED, lang->menu_normal_speed);
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_AUTO, ui(UI_MENU_AUTO));
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_SETTINGS, ui(UI_MENU_SETTINGS));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_STARTUP, ui(UI_MENU_STARTUP));
            CheckMenuItem(hMenu, ID_TRAY_STARTUP, MF_BYCOMMAND | (is_startup_enabled() ? MF_CHECKED : MF_UNCHECKED));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_ABOUT, lang->menu_about);
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, lang->menu_exit);

            switch (fan_speed_set_at_start) {
                case LOW_SPEED:
                    toggle_fan_low_speed();
                    break;
                case HIGH_SPEED:
                    toggle_fan_high_speed();
                    break;
                case NORMAL_SPEED:
                    toggle_fan_normal_speed();
                    break;
            }
            if (automatic) set_automatic(1, 0);
            else { start_temperature_monitor(); poll_temperature(); }
            break;
        }

        case WM_TRAYICON: {
            if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, 
                              pt.x, pt.y, 0, hwnd, NULL);
                PostMessage(hwnd, WM_NULL, 0, 0);
            }
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_TRAY_EXIT:
                    Shell_NotifyIcon(NIM_DELETE, &nid);
                    DestroyWindow(hwnd);
                    break;

                case ID_TRAY_LOW_SPEED:
                    set_automatic(0, 1);
                    toggle_fan_low_speed();
                    break;

                case ID_TRAY_HIGH_SPEED:
                    set_automatic(0, 1);
                    toggle_fan_high_speed();
                    break;

                case ID_TRAY_NORMAL_SPEED:
                    set_automatic(0, 1);
                    toggle_fan_normal_speed();
                    break;

                case ID_TRAY_AUTO:
                    if (automatic) { set_automatic(0, 1); toggle_fan_normal_speed(); }
                    else set_automatic(1, 1);
                    break;

                case ID_TRAY_SETTINGS:
                    DialogBoxW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_TEMPERATURE), hwnd, SettingsProc);
                    break;

                case ID_TRAY_ABOUT:
                    MessageBox(hwnd, lang->about_text, lang->menu_about, MB_OK | MB_ICONINFORMATION);
                    break;

                case ID_TRAY_STARTUP:
                    set_startup_enabled(!is_startup_enabled());
                    CheckMenuItem(hMenu, ID_TRAY_STARTUP, MF_BYCOMMAND | (is_startup_enabled() ? MF_CHECKED : MF_UNCHECKED));
                    break;
            }
            if (LOWORD(wParam) != ID_TRAY_EXIT) poll_temperature();
            break;
        }

        case WM_HOTKEY:
            set_automatic(0, 1);
            switch (wParam) {
                case HOTKEY_LOW_SPEED:
                    toggle_fan_low_speed();
                    break;

                case HOTKEY_HIGH_SPEED:
                    toggle_fan_high_speed();
                    break;

                case HOTKEY_NORMAL_SPEED:
                    toggle_fan_normal_speed();
                    break;
            }
            poll_temperature();
            break;

        case WM_TIMER:
            if (wParam == 1) poll_temperature();
            break;

        case WM_POWERBROADCAST:
            if (wParam == PBT_APMSUSPEND) {
                KillTimer(hwnd, 1);
                fan_worker_set(NORMAL_SPEED);
                temperature_stop();
            } else if (wParam == PBT_APMRESUMEAUTOMATIC) {
                if (automatic) set_automatic(1, 0);
                else {
                    fan_worker_set(current_speed);
                    start_temperature_monitor();
                    poll_temperature();
                }
            }
            return TRUE;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            fan_worker_stop();
            temperature_stop();
            DestroyMenu(hMenu);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    LANGID system_lang = PRIMARYLANGID(GetUserDefaultLangID());
    if (system_lang == LANG_SPANISH) { lang = &es; ui_language = 1; }
    else if (system_lang == LANG_CHINESE) { lang = &zh_CN; ui_language = 2; }

    int args;
    load_settings();
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &args);
    for (int i = 1; i < args; ++i) {
        if (wcscmp(argv[i], TEXT("--low-speed")) == 0) {
            automatic = 0;
            fan_speed_set_at_start = LOW_SPEED;
        } else if (wcscmp(argv[i], TEXT("--normal-speed")) == 0) {
            automatic = 0;
            fan_speed_set_at_start = NORMAL_SPEED;
        } else if (wcscmp(argv[i], TEXT("--high-speed")) == 0) {
            automatic = 0;
            fan_speed_set_at_start = HIGH_SPEED;
        } else if (wcscmp(argv[i], TEXT("--auto")) == 0) {
            automatic = 1;
        }
    }
    LocalFree(argv);

    HANDLE hMutex = CreateMutex(NULL, TRUE, TEXT("LenovoFanControlMutex"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        MessageBox(NULL, lang->program_is_running, lang->note, MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (read_state() == -1) {
        MessageBox(NULL, lang->failed_to_open_driver, lang->app_name, MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("LenovoFanControlClass");

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, ui(UI_REGISTER_ERROR), ui(UI_ERROR), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (!fan_worker_start()) return 1;
    HWND hwnd = CreateWindowEx(0, TEXT("LenovoFanControlClass"), lang->app_name, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL) {
        fan_worker_stop();
        MessageBox(NULL, ui(UI_WINDOW_ERROR), ui(UI_ERROR), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    RegisterHotKey(hwnd, HOTKEY_LOW_SPEED, MOD_CONTROL | MOD_ALT, VK_F10);
    RegisterHotKey(hwnd, HOTKEY_HIGH_SPEED, MOD_CONTROL | MOD_ALT, VK_F11);
    RegisterHotKey(hwnd, HOTKEY_NORMAL_SPEED, MOD_CONTROL | MOD_ALT, VK_F12);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterHotKey(hwnd, HOTKEY_LOW_SPEED);
    UnregisterHotKey(hwnd, HOTKEY_HIGH_SPEED);
    UnregisterHotKey(hwnd, HOTKEY_NORMAL_SPEED);

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
    return (int)msg.wParam;
}
