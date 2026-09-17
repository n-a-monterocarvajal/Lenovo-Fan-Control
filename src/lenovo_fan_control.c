#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <stdio.h>
#include <Windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <wchar.h>

#include "fanctrl.h"
#include "fan_worker.h"
#include "auto_control.h"
#include "temperature.h"
#include "ui_strings.h"
#include "../res/resource.h"

#define WM_TRAYICON (WM_USER + 1)

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

/* Same order as enum FanSpeed, so a hotkey ID is its speed. */
enum HotKeyIDs {
    HOTKEY_HIGH_SPEED,
    HOTKEY_LOW_SPEED,
    HOTKEY_NORMAL_SPEED,
};

NOTIFYICONDATA nid;
HMENU hMenu;
static enum FanSpeed current_speed = HIGH_SPEED;
static int automatic, high_threshold = 70, normal_threshold = 65, auto_high = 1;
static int elevation_declined;
static HANDLE hMutex;
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
        MessageBoxW(nid.hWnd, ui(UI_SAVE_ERROR), ui(UI_APP_NAME), MB_OK | MB_ICONWARNING);
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

/* poll_temperature() refreshes the menu and tooltip after every call. */
static void set_speed(enum FanSpeed speed) {
    current_speed = speed;
    fan_worker_set(speed);
}

static void poll_temperature(void) {
    double cpu = 0, gpu = -1;
    int valid = temperature_read(&cpu, &gpu);
    double celsius = cpu > gpu ? cpu : gpu;
    WCHAR label[160], state_label[80];
    LPCWSTR speed_label;
    if (automatic) {
        auto_high = auto_should_run_high(auto_high, celsius, valid, high_threshold, normal_threshold);
        set_speed(auto_high ? HIGH_SPEED : NORMAL_SPEED);
    }
    if (valid && gpu >= 0) swprintf(label, 160, ui(UI_TEMPERATURE_PAIR), cpu, gpu);
    else if (valid) swprintf(label, 160, ui(UI_CPU_ONLY), cpu);
    else wcscpy(label, ui(automatic ? UI_UNAVAILABLE_AUTO : UI_UNAVAILABLE_MANUAL));
    ModifyMenuW(hMenu, ID_TRAY_TEMPERATURE, MF_STRING | MF_DISABLED, ID_TRAY_TEMPERATURE, label);
    speed_label = current_speed == HIGH_SPEED ? ui(UI_AT_HIGH) :
        current_speed == LOW_SPEED ? ui(UI_AT_LOW) : ui(UI_AT_NORMAL);
    swprintf(state_label, 80, L"%ls (%ls)", speed_label, ui(automatic ? UI_AUTO : UI_MANUAL));
    ModifyMenuW(hMenu, ID_TRAY_STATE, MF_STRING | MF_DISABLED, ID_TRAY_STATE, state_label);
    swprintf(nid.szTip, 128, L"%.45ls\n%.80ls", state_label, label);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

static int start_temperature_monitor(void) {
    temperature_start();
    if (SetTimer(nid.hWnd, 1, 1000, NULL)) return 1;
    temperature_stop();
    MessageBoxW(nid.hWnd, ui(UI_TIMER_ERROR), ui(UI_APP_NAME), MB_OK | MB_ICONERROR);
    return 0;
}

/* EnergyDrv (fan control) works fine unelevated; only CPU MSR access via PawnIO
   needs Administrator. Relaunch elevated only when the user opts into Automatic
   mode, instead of forcing UAC on every launch just for manual speed control. */
static void relaunch_elevated(LPCWSTR extra_arg) {
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(NULL, path, MAX_PATH)) return;
    if (hMutex) ReleaseMutex(hMutex);
    if ((INT_PTR)ShellExecuteW(NULL, L"runas", path, extra_arg, NULL, SW_SHOWNORMAL) > 32)
        PostQuitMessage(0);
}

static void set_automatic(int enabled, int persist) {
    automatic = enabled;
    if (enabled) {
        double cpu, gpu;
        if (!IsUserAnAdmin() && !elevation_declined) {
            if (MessageBoxW(nid.hWnd, ui(UI_ELEVATE_PROMPT), ui(UI_APP_NAME), MB_YESNO | MB_ICONQUESTION) == IDYES) {
                relaunch_elevated(L"--auto");
                return;
            }
            elevation_declined = 1;
        }
        auto_high = 1;
        set_speed(HIGH_SPEED);
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
            wcscpy(nid.szTip, ui(UI_APP_NAME));
            Shell_NotifyIcon(NIM_ADD, &nid);

            hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, ID_TRAY_STATE, ui(UI_AT_HIGH));
            AppendMenuW(hMenu, MF_STRING | MF_DISABLED, ID_TRAY_TEMPERATURE, ui(UI_LOADING));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_LOW_SPEED, ui(UI_MENU_LOW));
            AppendMenu(hMenu, MF_STRING, ID_TRAY_HIGH_SPEED, ui(UI_MENU_HIGH));
            AppendMenu(hMenu, MF_STRING, ID_TRAY_NORMAL_SPEED, ui(UI_MENU_NORMAL));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_AUTO, ui(UI_MENU_AUTO));
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_SETTINGS, ui(UI_MENU_SETTINGS));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_STARTUP, ui(UI_MENU_STARTUP));
            CheckMenuItem(hMenu, ID_TRAY_STARTUP, MF_BYCOMMAND | (is_startup_enabled() ? MF_CHECKED : MF_UNCHECKED));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_ABOUT, ui(UI_MENU_ABOUT));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, ui(UI_MENU_EXIT));

            set_speed(current_speed);
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
                    set_speed(LOW_SPEED);
                    break;

                case ID_TRAY_HIGH_SPEED:
                    set_automatic(0, 1);
                    set_speed(HIGH_SPEED);
                    break;

                case ID_TRAY_NORMAL_SPEED:
                    set_automatic(0, 1);
                    set_speed(NORMAL_SPEED);
                    break;

                case ID_TRAY_AUTO:
                    if (automatic) { set_automatic(0, 1); set_speed(NORMAL_SPEED); }
                    else set_automatic(1, 1);
                    break;

                case ID_TRAY_SETTINGS:
                    DialogBoxW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_TEMPERATURE), hwnd, SettingsProc);
                    break;

                case ID_TRAY_ABOUT:
                    MessageBox(hwnd, ui(UI_ABOUT_TEXT), ui(UI_MENU_ABOUT), MB_OK | MB_ICONINFORMATION);
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
            set_speed((enum FanSpeed)wParam);
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
    if (system_lang == LANG_SPANISH) ui_language = 1;
    else if (system_lang == LANG_CHINESE) ui_language = 2;

    int args;
    load_settings();
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &args);
    for (int i = 1; i < args; ++i) {
        if (wcscmp(argv[i], L"--low-speed") == 0) {
            automatic = 0;
            current_speed = LOW_SPEED;
        } else if (wcscmp(argv[i], L"--normal-speed") == 0) {
            automatic = 0;
            current_speed = NORMAL_SPEED;
        } else if (wcscmp(argv[i], L"--high-speed") == 0) {
            automatic = 0;
            current_speed = HIGH_SPEED;
        } else if (wcscmp(argv[i], L"--auto") == 0) {
            automatic = 1;
        }
    }
    LocalFree(argv);

    hMutex = CreateMutex(NULL, TRUE, L"LenovoFanControlMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        MessageBox(NULL, ui(UI_ALREADY_RUNNING), ui(UI_NOTE), MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (read_state() == -1) {
        MessageBox(NULL, ui(UI_DRIVER_ERROR), ui(UI_APP_NAME), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LenovoFanControlClass";

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, ui(UI_REGISTER_ERROR), ui(UI_ERROR), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (!fan_worker_start()) return 1;
    HWND hwnd = CreateWindowEx(0, L"LenovoFanControlClass", ui(UI_APP_NAME), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
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

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
    return (int)msg.wParam;
}
