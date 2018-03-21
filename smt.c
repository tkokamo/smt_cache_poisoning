#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <time.h>

#define RDTSC(x) \
	asm volatile("rdtsc; shlq $32, %%rdx; orq %%rdx, %%rax" : "=a" (x) :: "%rdx")

#define BUFSIZE 1024*32
#define ITER	1024

struct smt_data {
	unsigned long	start, end;
	char		*buf;
	cpu_set_t	cpu_set;
};

void* loop(void *arg)
{
	struct smt_data *data = (struct smt_data *)arg;
	char *buf = data->buf;
	int i, j;

	srand((unsigned)time(NULL));

	sched_setaffinity(getpid(), sizeof(cpu_set_t), &data->cpu_set);

	/* warm up */
	for (i = 0; i < ITER; i++) {
		for (j = 0; j < BUFSIZE; j++) {
			buf[j] = j % 8;
		}
	}

	RDTSC(data->start);
	for (i = 0; i < ITER; i++) {
		for (j = 0; j < BUFSIZE; j++) {
			buf[rand() % BUFSIZE] = j % 8;
		}
	}
	RDTSC(data->end);

	pthread_exit(arg);
}

int main(void)
{
	pthread_t thr1, thr2;
	struct smt_data thr1_data, thr2_data;
	cpu_set_t cpu_set;
	unsigned long sum;
	void *ret;

	thr1_data.buf = (char *)malloc(sizeof(char)*BUFSIZE);
	thr2_data.buf = (char *)malloc(sizeof(char)*BUFSIZE);

	CPU_ZERO(&thr1_data.cpu_set);
	CPU_SET(6, &thr1_data.cpu_set);
	CPU_ZERO(&thr2_data.cpu_set);
	CPU_SET(7, &thr2_data.cpu_set);

	pthread_create(&thr1, NULL, loop, (void *)&thr1_data);
	pthread_create(&thr2, NULL, loop, (void *)&thr2_data);

	pthread_join(thr1, &ret);
	pthread_join(thr2, &ret);

	sum = thr1_data.end - thr1_data.start;
	sum += thr2_data.end - thr2_data.start;

	printf("%ld elapsed.\n", sum);
	return 0;
}
