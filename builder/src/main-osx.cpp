#include <iostream>
#include <putki/builder/typereg.h>

// Putki data builder.

void bind_app_types();

int main(int argc, char **argv)
{
//	putki::typereg_register("bajs", 0);
	bind_app_types();

	return 0;
}

