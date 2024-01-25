#include <xinu.h>
#include <prodcons_bb.h>

void producer_bb(int count)
{
	for(int i=0; i<count; i++)
	{
		wait(write_mutex);
		arr[tail % 5] = i;
		printf("name : %s, write : %d\n", proctab[getpid()].prname, arr[tail%5]);
		tail++;
		signal(read_mutex);
	}
}
