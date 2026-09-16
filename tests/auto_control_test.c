#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "../src/auto_control.h"

int main(void) {
    int state = 0;
    assert(auto_thresholds_valid(70, 65));
    assert(auto_thresholds_valid(100, 20));
    assert(!auto_thresholds_valid(65, 65));
    assert(!auto_thresholds_valid(60, 65));
    assert(!auto_thresholds_valid(101, 65));
    assert(!auto_thresholds_valid(70, 19));
    /* Rising and falling traces, including equality and the dead band. */
    double trace[] = { 50, 69.9, 70, 72, 69, 65.1, 65, 67, 70 };
    int expected[] = { 0, 0, 1, 1, 1, 1, 0, 0, 1 };
    for (int i = 0; i < 9; ++i) {
        state = auto_should_run_high(state, trace[i], 1, 70, 65);
        assert(state == expected[i]);
    }
    /* A failed/stale sample must never be interpreted as a cool machine. */
    assert(auto_should_run_high(0, 0, 0, 70, 65));
    assert(auto_should_run_high(0, NAN, 1, 70, 65));
    assert(auto_should_run_high(0, INFINITY, 1, 70, 65));
    assert(auto_should_run_high(0, -1, 1, 70, 65));
    assert(auto_should_run_high(0, 151, 1, 70, 65));
    assert(auto_should_run_high(0, 30, 1, 60, 65));
    assert(auto_should_run_high(1, 67, 1, 70, 65));
    assert(!auto_should_run_high(1, 60, 1, 70, 65));
    puts("Temperature policy tests passed.");
    return 0;
}
