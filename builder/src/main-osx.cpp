#include <iostream>
#include <fstream>
#include <string>

// Putki data builder.

int databuilder_main(int argc, char **argv);
void app_bind_putki_types();

int main(int argc, char **argv)
{
	app_bind_putki_types();
	return databuilder_main(argc, argv);
}
