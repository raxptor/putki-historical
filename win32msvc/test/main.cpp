#include <putki/cpp-runtime/resolve.h>
#include <putki/builder-config.h>

int main (int argc, char *argv[])
{
	putki::resolver_alloc(65536);

	return 0;
}
