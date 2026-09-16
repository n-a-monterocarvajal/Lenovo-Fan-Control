#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "../src/fanctrl.h"
#include "../src/fan_worker.h"

static volatile LONG calls, mode = -1, active, overlap, stuck;
enum FanMode read_state(void) { return (enum FanMode)InterlockedCompareExchange(&mode, 0, 0); }
int fan_control(enum FanMode next) {
    if (InterlockedIncrement(&active) != 1) InterlockedIncrement(&overlap);
    Sleep(1);
    if (!stuck) InterlockedExchange(&mode, next);
    InterlockedIncrement(&calls);
    InterlockedDecrement(&active);
    return 1;
}
static void await_mode(LONG expected) {
    DWORD start = GetTickCount();
    while (InterlockedCompareExchange(&mode, 0, 0) != expected && GetTickCount() - start < 2000) Sleep(5);
    assert(InterlockedCompareExchange(&mode, 0, 0) == expected);
}
int main(void) {
    assert(fan_worker_start());
    await_mode(NORMAL);
    fan_worker_set(HIGH_SPEED);
    await_mode(FAST);
    LONG before = InterlockedCompareExchange(&calls, 0, 0);
    for (int i = 0; i < 20; ++i) fan_worker_set(HIGH_SPEED);
    Sleep(50);
    assert(InterlockedCompareExchange(&calls, 0, 0) == before);
    for (int i = 0; i < 100; ++i) {
        fan_worker_set(LOW_SPEED);
        fan_worker_set(HIGH_SPEED);
        fan_worker_set(NORMAL_SPEED);
    }
    await_mode(NORMAL);
    fan_worker_set(HIGH_SPEED);
    await_mode(FAST);
    fan_worker_stop();
    assert(mode == NORMAL && overlap == 0);
    before = calls;
    Sleep(50);
    assert(calls == before);
    assert(fan_worker_start());
    InterlockedExchange(&stuck, 1);
    fan_worker_set(HIGH_SPEED);
    Sleep(50);
    DWORD stop_started = GetTickCount();
    fan_worker_stop();
    assert(GetTickCount() - stop_started < 1000);
    puts("Fan worker tests passed (mock driver; no hardware access).");
    return 0;
}
