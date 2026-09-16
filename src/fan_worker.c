#include <Windows.h>
#include "fanctrl.h"
#include "fan_worker.h"

static HANDLE changed, worker;
static volatile LONG requested = NORMAL_SPEED;
static volatile LONG stopping;

static int apply_mode(enum FanMode mode, LONG speed) {
    while (!InterlockedCompareExchange(&stopping, 0, 0) &&
           InterlockedCompareExchange(&requested, 0, 0) == speed) {
        enum FanMode actual;
        if (fan_control(mode) == -1) return 0;
        actual = read_state();
        if (actual == mode) return 1;
        if (actual == (enum FanMode)-1) return 0;
        WaitForSingleObject(changed, 10);
    }
    return 0;
}

/* One owner of the driver: rapid manual/automatic transitions cannot leave
 * overlapping old workers writing NORMAL after a new HIGH request. */
static DWORD WINAPI run(LPVOID unused) {
    (void)unused;
    while (!InterlockedCompareExchange(&stopping, 0, 0)) {
        LONG speed = InterlockedCompareExchange(&requested, 0, 0);
        DWORD delay = INFINITE;
        int reset_ok = apply_mode(NORMAL, speed);
        if (speed == HIGH_SPEED) {
            /* Preserve the original read-back/retry behavior, but allow a mode
             * change to interrupt it even when the driver stops responding. */
            delay = reset_ok && apply_mode(FAST, speed) ? 8980 : 1000;
        } else if (speed == LOW_SPEED) {
            delay = 5000;
        } else if (!reset_ok) {
            delay = 1000;
        }
        if (InterlockedCompareExchange(&requested, 0, 0) != speed ||
            InterlockedCompareExchange(&stopping, 0, 0)) continue;
        WaitForSingleObject(changed, delay);
    }
    fan_control(NORMAL);
    return 0;
}

int fan_worker_start(void) {
    InterlockedExchange(&stopping, 0);
    InterlockedExchange(&requested, NORMAL_SPEED);
    changed = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!changed) return 0;
    worker = CreateThread(NULL, 0, run, NULL, 0, NULL);
    if (!worker) { CloseHandle(changed); changed = NULL; }
    return worker != NULL;
}

void fan_worker_set(enum FanSpeed speed) {
    if (InterlockedExchange(&requested, speed) != (LONG)speed)
        SetEvent(changed);
}

void fan_worker_stop(void) {
    if (!worker) return;
    InterlockedExchange(&stopping, 1);
    SetEvent(changed);
    WaitForSingleObject(worker, INFINITE);
    CloseHandle(worker);
    CloseHandle(changed);
    worker = changed = NULL;
}
