#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

static inline int sys_perf_event_open(struct perf_event_attr *attr, pid_t pid,
				      int cpu, int group_fd,
				      unsigned long flags)
{
	attr->size = sizeof(*attr);
	return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

static int cycles_fd;
static int instructions_fd;

void close_pmu_counters(void)
{
	close(cycles_fd);
	close(instructions_fd);
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
		perror("sys_perf_event_open cpu cycles");
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
		perror("sys_perf_event_open hw instructions");
		return -1;
	}
	return 0;
}

void start_pmu_counters(void)
{
	/* Only need to start and stop the group leader */
	ioctl(instructions_fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(cycles_fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(cycles_fd, PERF_EVENT_IOC_ENABLE);
}

void stop_pmu_counters(void)
{
	ioctl(cycles_fd, PERF_EVENT_IOC_DISABLE);
}

void print_pmu_counters(void)
{
	size_t res;
	unsigned long long cycles;
	unsigned long long instructions;

	res = read(cycles_fd, &cycles, sizeof(unsigned long long));
	assert(res == sizeof(unsigned long long));

	res = read(instructions_fd, &instructions, sizeof(unsigned long long));
	assert(res == sizeof(unsigned long long));

	printf("cycles:\t\t%lld\n", cycles);
	printf("instructions:\t%lld\n", instructions);
	if (instructions > 0)
		printf("CPI:\t\t%0.2f\n", (float)cycles/instructions);
}
