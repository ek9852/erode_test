#ifndef _PMU_UTILS_H
#define _PMU_UTILS_H

int setup_pmu_counters(void);
void start_pmu_counters(void);
void stop_pmu_counters(void);
void close_pmu_counters(void);
void print_pmu_counters(void);

#endif
