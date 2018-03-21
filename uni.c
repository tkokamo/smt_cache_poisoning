#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>

#define RDTSC(x) \
	asm volatile("rdtsc; shlq $32, %%rdx; orq %%rdx, %%rax" : "=a" (x) :: "%rdx")

#define BUFSIZE 1024*32
#define ITER	1024

int main(void)
{
	char *buf1, *buf2;
	int i, j;
	unsigned long start, end;
	unsigned long sum;
	cpu_set_t cpu_set;

	buf1 = (char *)malloc(sizeof(*buf1)*BUFSIZE);
	buf2 = (char *)malloc(sizeof(*buf2)*BUFSIZE);

	srand((unsigned)time(NULL));

	CPU_ZERO(&cpu_set);
	CPU_SET(7, &cpu_set);
	sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpu_set);

	/* warm up */
	for (i = 0; i < ITER; i++) {
		for (j = 0; j < BUFSIZE; j++) {
			buf1[j] = j % 8;
		}
	}
	
	RDTSC(start);
	for (i = 0; i < ITER; i++) {
		for (j = 0; j < BUFSIZE; j++) {
			buf1[rand() % BUFSIZE] = j % 8;
		}
	}
	RDTSC(end);
	sum = end - start;

	/* warm up */
	for (i = 0; i < ITER; i++) {
		for (j = 0; j < BUFSIZE; j++) {
			buf2[j] = j % 8;
		}
	}

	RDTSC(start);
	for (i = 0; i < ITER; i++) {
		for (j = 0; j < BUFSIZE; j++) {
			buf2[rand() % BUFSIZE] = j % 8;
		}
	}
	RDTSC(end);
	sum += end - start;

	printf("%ld elapsed\n", sum);
	return 0;
}
