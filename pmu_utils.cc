#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#endif

static timespec start, end;

#ifdef __linux__
static inline int sys_perf_event_open(struct perf_event_attr *attr, pid_t pid,
				      int cpu, int group_fd,
				      unsigned long flags)
{
    attr->size = sizeof(*attr);
    return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

static int cycles_fd;
static int instructions_fd;
static bool pmu_ready = false;

void close_pmu_counters(void)
{
    if (pmu_ready) {
        close(cycles_fd);
        close(instructions_fd);
    }
    pmu_ready = false;
}

int setup_pmu_counters(void)
{
    struct perf_event_attr attr;

    memset(&attr, 0, sizeof(attr));
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    attr.disabled = 1;
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = PERF_COUNT_HW_CPU_CYCLES;
    cycles_fd = sys_perf_event_open(&attr, 0, -1, -1, 0);
    if (cycles_fd < 0) {
        perror("pmu counter not available. sys_perf_event_open cpu cycles");
        return -1;
    }

    /*
     * We use cycles_fd as the group leader in order to ensure
     * both counters run at the same time and our CPI statistics are
     * valid.
     */
    attr.disabled = 0; /* The group leader will start/stop us */
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = PERF_COUNT_HW_INSTRUCTIONS;
    instructions_fd = sys_perf_event_open(&attr, 0, -1, cycles_fd, 0);
    if (instructions_fd < 0) {
        perror("pmu counter not available. sys_perf_event_open hw instructions");
        close(cycles_fd);
        return -1;
    }
    pmu_ready = true;
    return 0;
}

void start_pmu_counters(void)
{
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (pmu_ready) {
        /* Only need to start and stop the group leader */
        ioctl(instructions_fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(cycles_fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(instructions_fd, PERF_EVENT_IOC_ENABLE);
        ioctl(cycles_fd, PERF_EVENT_IOC_ENABLE);
    }
}

void stop_pmu_counters(void)
{
    if (pmu_ready)
        ioctl(cycles_fd, PERF_EVENT_IOC_DISABLE);
    clock_gettime(CLOCK_MONOTONIC, &end);
}

void print_pmu_counters(void)
{
    size_t res;
    unsigned long long cycles;
    unsigned long long instructions;

    if (pmu_ready) {
        res = read(cycles_fd, &cycles, sizeof(unsigned long long));
        assert(res == sizeof(unsigned long long));

        res = read(instructions_fd, &instructions, sizeof(unsigned long long));
        assert(res == sizeof(unsigned long long));

        printf("cycles:\t\t%lld\n", cycles);
        printf("instructions:\t%lld\n", instructions);
        if (instructions > 0)
            printf("CPI:\t\t%0.2f\n", (float)cycles/instructions);
    }

    int64_t start_ns;
    int64_t end_ns;
    int64_t diff_ns;

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;
}
#else
int setup_pmu_counters(void)
{
    std::cout << "No PMU counter available" << std::endl;
    return 0;
}

void start_pmu_counters(void)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
}

void stop_pmu_counters(void)
{
    clock_gettime(CLOCK_MONOTONIC, &end);
}

void close_pmu_counters(void)
{
}

void print_pmu_counters(void)
{
    int64_t start_ns;
    int64_t end_ns;
    int64_t diff_ns;

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;
}
#endif
