#ifndef AUTO_CONTROL_H
#define AUTO_CONTROL_H

/* Celsius; a lower release threshold prevents oscillation. */
int auto_thresholds_valid(int high, int normal);
int auto_should_run_high(int was_high, double temperature, int valid,
                         int high, int normal);
#endif
