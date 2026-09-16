#define UNICODE
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "temperature.h"

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
    DWORD length;
    temperature_stop();
    length = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (!length || length >= MAX_PATH || !wcsrchr(path, L'\\')) return 0;
    *wcsrchr(path, L'\\') = 0;
    if (wcslen(path) + 25 >= MAX_PATH) return 0;
    wcscat(path, L"\\TemperatureMonitor.exe");
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
