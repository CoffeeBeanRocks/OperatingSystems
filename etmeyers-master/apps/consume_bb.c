#include <xinu.h>
#include <prodcons_bb.h>
#include <run.h>

void consumer_bb(int count)
{
	for(int i=0; i<count; i++)
	{
		wait(read_mutex);
		printf("name : %s, read : %d\n", proctab[getpid()].prname, arr[head%5]);
		head++;
		signal(write_mutex);
	}
	signal(run_mutex);
}
