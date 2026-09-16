#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "../src/temperature.h"

static void start(const char *scenario) {
    SetEnvironmentVariableA("LFC_TEST_SCENARIO", scenario);
    assert(temperature_start());
}
static void expect_sample(double expected_cpu, double expected_gpu) {
    double cpu = 0, gpu = 0;
    DWORD start = GetTickCount();
    int valid;
    do { Sleep(20); valid = temperature_read(&cpu, &gpu); }
    while (!valid && GetTickCount() - start < 3000);
    assert(valid && cpu == expected_cpu && gpu == expected_gpu);
}
int main(void) {
    double cpu, gpu;
    const char *invalid[] = { "invalid", "nan", "extra", "overflow", "exit" };
    start("valid"); expect_sample(71.5, 63.0); temperature_stop();
    assert(!temperature_read(&cpu, &gpu));
    start("cpu-only"); expect_sample(61.5, -1); temperature_stop();
    start("fragmented"); expect_sample(71.5, 63.0); temperature_stop();
    for (int i = 0; i < 5; ++i) {
        start(invalid[i]); Sleep(200);
        assert(!temperature_read(&cpu, &gpu)); temperature_stop();
    }
    start("valid"); expect_sample(71.5, 63.0);
    Sleep(10100);
    assert(!temperature_read(&cpu, &gpu)); temperature_stop();
    /* Restart cannot reuse a previous cool sample. */
    start("silent");
    assert(!temperature_read(&cpu, &gpu)); temperature_stop();
    puts("Sensor IPC tests passed (simulated readings, no hardware access).");
    return 0;
}
