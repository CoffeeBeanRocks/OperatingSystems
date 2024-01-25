#include <xinu.h>
#include <prodcons.h>

void consumer(int count) {
  // TODO: implement the following:
  // - Iterates from 0 to count (count including)
  //   - reading the value of the global variable 'n' each time
  //   - print consumed value (the value of 'n'), e.g. "consumed : 8"

	for(int i=count; i>=0; i--)
	{
		wait(r_mutex);
		printf("consumed : %d\n", n);
		signal(w_mutex);
	}

	signal(p_mutex);
}
