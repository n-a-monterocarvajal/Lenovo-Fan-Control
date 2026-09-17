/* Exercise the real tray handlers and dialog resources with a fake sensor/driver.
 * No tray icon, message box or physical fan is touched. */
#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <shellapi.h>
#include <assert.h>
#include <stdio.h>

static unsigned errors;
static BOOL fake_notify(DWORD command, PNOTIFYICONDATAW data) {
    (void)command; (void)data; return TRUE;
}
static int fake_message(HWND owner, LPCWSTR text, LPCWSTR caption, UINT flags) {
    (void)owner; (void)text; (void)caption; (void)flags; ++errors; return IDOK;
}
#define Shell_NotifyIconW fake_notify
#define MessageBoxW fake_message
#define WinMain application_main
#include "../src/lenovo_fan_control.c"
#undef WinMain

static volatile LONG mock_mode;
int fan_control(enum FanMode mode) { InterlockedExchange(&mock_mode, mode); return 1; }
enum FanMode read_state(void) { return (enum FanMode)InterlockedCompareExchange(&mock_mode, 0, 0); }

int main(void) {
    WNDCLASSW cls = {0};
    cls.lpfnWndProc = WndProc;
    cls.hInstance = GetModuleHandleW(NULL);
    cls.lpszClassName = L"TrayUiTest";
    assert(RegisterClassW(&cls));
    SetEnvironmentVariableA("LFC_TEST_SCENARIO", "valid");
    for (int language = 0; language < 2; ++language) {
        WCHAR text[256];
        double cpu, gpu;
        ui_language = language;
        automatic = 0;
        elevation_declined = 1; /* skip the UAC-relaunch prompt; not what this test covers */
        high_threshold = 70; normal_threshold = 65;
        current_speed = NORMAL_SPEED;
        GetFullPathNameW(L"test-results\\ui-settings.ini", MAX_PATH, settings_path, NULL);
        assert(fan_worker_start());
        HWND window = CreateWindowW(cls.lpszClassName, L"Test", 0, 0, 0, 0, 0,
                                    NULL, NULL, cls.hInstance, NULL);
        assert(window);
        DWORD started = GetTickCount();
        while (!temperature_read(&cpu, &gpu) && GetTickCount() - started < 3000) Sleep(20);
        assert(temperature_read(&cpu, &gpu));
        poll_temperature();
        assert(GetMenuItemID(hMenu, 0) == ID_TRAY_STATE);
        assert(GetMenuItemID(hMenu, 1) == ID_TRAY_TEMPERATURE);
        GetMenuStringW(hMenu, ID_TRAY_TEMPERATURE, text, 256, MF_BYCOMMAND);
        assert(wcsstr(text, L"71.5") && wcsstr(text, L"63.0"));
        GetMenuStringW(hMenu, ID_TRAY_AUTO, text, 256, MF_BYCOMMAND);
        assert(!wcscmp(text, ui(UI_MENU_AUTO)));
        SendMessageW(window, WM_COMMAND, ID_TRAY_LOW_SPEED, 0);
        poll_temperature();
        assert(!automatic && current_speed == LOW_SPEED); /* Manual means manual. */
        SendMessageW(window, WM_COMMAND, ID_TRAY_AUTO, 0);
        assert(automatic && current_speed == HIGH_SPEED);
        HWND dialog = CreateDialogW(cls.hInstance, MAKEINTRESOURCEW(IDD_TEMPERATURE), window, SettingsProc);
        assert(dialog);
        GetDlgItemTextW(dialog, IDOK, text, 256);
        assert(!wcscmp(text, language ? L"Guardar" : L"Save"));
        SetDlgItemInt(dialog, IDC_HIGH_TEMP, 60, FALSE);
        SetDlgItemInt(dialog, IDC_NORMAL_TEMP, 65, FALSE);
        SendMessageW(dialog, WM_COMMAND, IDOK, 0);
        assert(high_threshold == 70 && normal_threshold == 65 && errors == 1);
        SetDlgItemInt(dialog, IDC_HIGH_TEMP, 80, FALSE);
        SetDlgItemInt(dialog, IDC_NORMAL_TEMP, 75, FALSE);
        SendMessageW(dialog, WM_COMMAND, IDOK, 0);
        assert(high_threshold == 80 && normal_threshold == 75 && current_speed == NORMAL_SPEED);
        /* Widen the band around the current 71.5C reading while auto_high is stale
           at HIGH: the new band (50-90) shouldn't blindly keep that old verdict. */
        auto_high = 1;
        set_speed(HIGH_SPEED);
        assert(current_speed == HIGH_SPEED);
        SetDlgItemInt(dialog, IDC_HIGH_TEMP, 90, FALSE);
        SetDlgItemInt(dialog, IDC_NORMAL_TEMP, 50, FALSE);
        SendMessageW(dialog, WM_COMMAND, IDOK, 0);
        assert(current_speed == NORMAL_SPEED); /* Editing thresholds re-decides immediately. */
        assert(wcsstr(ui(UI_ABOUT_TEXT), L"n-a-monterocarvajal"));
        /* The menu item persists the choice; the relaunch itself is not exercised. */
        always_elevated = 0;
        SendMessageW(window, WM_COMMAND, ID_TRAY_ELEVATED, 0);
        assert(always_elevated && GetPrivateProfileIntW(L"Startup", L"Elevated", 0, settings_path) == 1);
        SendMessageW(window, WM_COMMAND, ID_TRAY_ELEVATED, 0);
        assert(!always_elevated && GetPrivateProfileIntW(L"Startup", L"Elevated", 1, settings_path) == 0);
        DestroyWindow(dialog);
        SendMessageW(window, WM_COMMAND, ID_TRAY_NORMAL_SPEED, 0);
        assert(!automatic && temperature_read(&cpu, &gpu));
        DestroyWindow(window);
        errors = 0;
    }
    puts("English/Spanish tray, manual monitoring and threshold dialog tests passed.");
    return 0;
}
