
#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <run.h>

shellcmd xsh_hello(int argc, char* argv[])
{
	if(argc == 2 && strncmp(argv[1], "--help", 7) == 0)
	{
		printf("Usage: %s {YOUR NAME}\n\n", argv[0]);
		printf("Description:\nDisplays hello followed by your name\n\n");
		printf("Options (one per invocation):\n");
		printf("--help\tdisplay this help and exit\n");
		return 0;
	}
	else if(argc > 2)
	{
		fprintf(stderr, "Too many arguments\n");
		return 1;
	}
	else if(argc < 2)
	{
		fprintf(stderr, "Too few arguments\n");
		return 1;
	}

	printf("Hello %s, Welcome to the world of Xinu!!\n", argv[1]);

	signal(run_mutex);
	return 0;
}
