#include <iostream>

#include <outki/types/core.h>
#include <putki/pkgloader.h>
#include <putki/pkgmgr.h>

#include <stdio.h>

// binding up the blob loads.
namespace outki { void bind_test_project_loaders(); }

int main(int argc, char *argv[])
{
	std::cout << "Test-App launching!" << std::endl;
	outki::bind_test_project_loaders();

	outki::pkgmgr::loaded_package *pkg = outki::pkgloader::from_file("output/win32/packages/everything.pkg");
	if (!pkg)
	{
		std::cout << "Failed loading everything pkg!" << std::endl;
	}
	
	std::cout << "Loaded package! " << pkg << std::endl;

	outki::pkgmgr::release(pkg);
	
	return 0;
}
