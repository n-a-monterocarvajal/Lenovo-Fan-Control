#define UNICODE
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "temperature.h"
#include "../res/resource.h"

/* TemperatureMonitor.exe is embedded as an RCDATA resource so the release build ships
   as a single file; extract it to a temp path once per run and launch it from there.
   Test binaries carry no such resource and fall back to a sibling exe (tests/fake_sensor.c). */
static int extract_sensor_exe(WCHAR *path, size_t path_capacity) {
    HRSRC res;
    HGLOBAL loaded;
    void *data;
    DWORD size, written, length;
    HANDLE file;
    WCHAR dir[MAX_PATH];

    res = FindResourceW(NULL, MAKEINTRESOURCE(IDR_SENSOR_EXE), RT_RCDATA);
    if (!res) {
        length = GetModuleFileNameW(NULL, path, (DWORD)path_capacity);
        if (!length || length >= path_capacity || !wcsrchr(path, L'\\')) return 0;
        *wcsrchr(path, L'\\') = 0;
        if (wcslen(path) + 25 >= path_capacity) return 0;
        wcscat(path, L"\\TemperatureMonitor.exe");
        return 1;
    }
    loaded = LoadResource(NULL, res);
    if (!loaded) return 0;
    data = LockResource(loaded);
    size = SizeofResource(NULL, res);
    if (!data || !size) return 0;

    if (!GetTempPathW(MAX_PATH, dir)) return 0;
    if (_snwprintf(path, path_capacity, L"%lsLenovoFanControl-Sensor.exe", dir) < 0) return 0;
    file = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return 0;
    written = 0;
    if (!WriteFile(file, data, size, &written, NULL) || written != size) { CloseHandle(file); return 0; }
    CloseHandle(file);
    return 1;
}

static HANDLE process, output, stop_event;
static char line[64];
static unsigned used;
static DWORD last_sample;
static double latest_cpu, latest_gpu;
static int valid;
static int overflow;

int temperature_start(void) {
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE writer = NULL, parent = NULL;
    WCHAR path[MAX_PATH], command[MAX_PATH + 100];
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};
    temperature_stop();
    if (!extract_sensor_exe(path, MAX_PATH)) return 0;
    if (!CreatePipe(&output, &writer, &sa, 0)) goto failed;
    if (!SetHandleInformation(output, HANDLE_FLAG_INHERIT, 0)) goto failed;
    stop_event = CreateEventW(&sa, TRUE, FALSE, NULL);
    if (!stop_event) goto failed;
    parent = OpenProcess(SYNCHRONIZE, TRUE, GetCurrentProcessId());
    if (!parent) goto failed;
    swprintf(command, sizeof(command) / sizeof(*command), L"\"%ls\" %llu %llu",
             path, (unsigned long long)(ULONG_PTR)stop_event, (unsigned long long)(ULONG_PTR)parent);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = writer;
    si.hStdError = writer;
    if (!CreateProcessW(path, command, NULL, NULL, TRUE, CREATE_NO_WINDOW,
                        NULL, NULL, &si, &pi)) goto failed;
    CloseHandle(writer);
    CloseHandle(parent);
    CloseHandle(pi.hThread);
    process = pi.hProcess;
    return 1;
failed:
    if (parent) CloseHandle(parent);
    if (writer) CloseHandle(writer);
    temperature_stop();
    return 0;
}

int temperature_read(double *cpu, double *gpu) {
    DWORD available, count;
    char buffer[256];
    if (!process || WaitForSingleObject(process, 0) != WAIT_TIMEOUT) return 0;
    /* Nonblocking: sensor initialization/reads never delay the fan worker/UI. */
    while (PeekNamedPipe(output, NULL, 0, NULL, &available, NULL) && available) {
        if (!ReadFile(output, buffer, min(available, sizeof(buffer)), &count, NULL)) break;
        for (DWORD i = 0; i < count; ++i) {
            if (buffer[i] == '\n') {
                char extra;
                line[used] = 0;
                valid = !overflow && sscanf(line, "%lf %lf %c", &latest_cpu, &latest_gpu, &extra) == 2 &&
                    latest_cpu >= 0 && latest_cpu <= 150 &&
                    (latest_gpu == -1 || (latest_gpu >= 0 && latest_gpu <= 150));
                last_sample = GetTickCount();
                used = 0;
                overflow = 0;
            } else if (buffer[i] != '\r') {
                if (used < sizeof(line) - 1) line[used++] = buffer[i];
                else { valid = 0; overflow = 1; }
            }
        }
    }
    *cpu = latest_cpu;
    *gpu = latest_gpu;
    return valid && (DWORD)(GetTickCount() - last_sample) < 10000;
}

void temperature_stop(void) {
    if (stop_event) SetEvent(stop_event);
    if (process) {
        if (WaitForSingleObject(process, 3000) == WAIT_TIMEOUT)
            TerminateProcess(process, 1);
        CloseHandle(process);
    }
    if (output) CloseHandle(output);
    if (stop_event) CloseHandle(stop_event);
    process = output = stop_event = NULL;
    valid = 0;
    used = 0;
    overflow = 0;
}
