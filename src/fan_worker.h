#ifndef FAN_WORKER_H
#define FAN_WORKER_H
enum FanSpeed { HIGH_SPEED, LOW_SPEED, NORMAL_SPEED };
int fan_worker_start(void);
void fan_worker_set(enum FanSpeed speed);
void fan_worker_stop(void);
#endif
