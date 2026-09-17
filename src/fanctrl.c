/* A fan control application for Lenovo laptops, e.g. Ideapad, Xiaoxin and etc.
 * 
 * References:
 * https://github.com/bitrate16/FanControl/blob/main/FanControl/FanControl.cpp
 * https://github.com/Soberia/Lenovo-IdeaPad-Z500-Fan-Controller?tab=readme-ov-file#-about
 * https://www.allstone.lt/ideafan/
 */

#include <Windows.h>

#include "fanctrl.h"

static int NORMAL_MODE_EXPECTED_VALUE = -1;

int fan_control(enum FanMode mode) {
    HANDLE hndl = CreateFileW(L"\\\\.\\EnergyDrv", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hndl == INVALID_HANDLE_VALUE) {
        return -1;
    }
    // lpInBuffer value: 06 00 00 00  01 00 00 00  01 00 00 00 ~ [ 6, 1, 1 ] (inv endian)
    DWORD inBuffer[3] = { 6, 1 };
    inBuffer[2] = mode;
    DWORD bytesReturned = 0;

    BOOL succeeded = DeviceIoControl(hndl, 0x831020C0, inBuffer, sizeof(inBuffer), NULL, 0, &bytesReturned, NULL);
    CloseHandle(hndl);

    return succeeded ? 1 : -1;
}

enum FanMode read_state() {
    if (NORMAL_MODE_EXPECTED_VALUE == -1) {
        // Set fan spinning mode to NORMAL to get NORMAL_MODE_EXPECTED_VALUE at the first run
        if (fan_control(NORMAL) == -1) return -1;
        Sleep(50);
    }
    HANDLE hndl = CreateFileW(L"\\\\.\\EnergyDrv", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hndl == INVALID_HANDLE_VALUE) {
        return -1;
    }
    // lpInBuffer value: 0E 00 00 00 ~ [ 14 ] (inv endian)
    DWORD inBuffer[1] = { 14 };
    DWORD outBuffer[1];
    DWORD bytesReturned = 0;

    BOOL succeeded = DeviceIoControl(hndl, 0x831020C4, inBuffer, sizeof(inBuffer), outBuffer, sizeof(outBuffer), &bytesReturned, NULL);
    CloseHandle(hndl);
    if (!succeeded || bytesReturned < sizeof(outBuffer)) return -1;

    if (NORMAL_MODE_EXPECTED_VALUE == -1) {
        // Set this value when the fan is in normal mode at the first run
        NORMAL_MODE_EXPECTED_VALUE = outBuffer[0];
    }
    return outBuffer[0] == (DWORD)NORMAL_MODE_EXPECTED_VALUE ? NORMAL : FAST;
}
