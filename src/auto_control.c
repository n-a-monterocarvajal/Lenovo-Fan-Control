#include "auto_control.h"

int auto_thresholds_valid(int high, int normal) {
    return normal >= 20 && high <= 100 && normal < high;
}

int auto_should_run_high(int was_high, double temperature, int valid,
                         int high, int normal) {
    if (!valid || !(temperature >= 0 && temperature <= 150) ||
        !auto_thresholds_valid(high, normal)) return 1;
    if (temperature >= high) return 1;
    if (temperature <= normal) return 0;
    return was_high;
}
