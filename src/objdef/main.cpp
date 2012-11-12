#include <compiler/compile.h>

int main (int argc, char *argv[])
{
	if (argc > 1)
	{
		putki::compile(argv[1]);
	}

	return 0;
}
