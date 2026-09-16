#ifndef TEMPERATURE_H
#define TEMPERATURE_H
int temperature_start(void);
int temperature_read(double *cpu, double *gpu);
void temperature_stop(void);
#endif
