#include <iostream>

#include <outki/types/core.h>
#include <outki/types/test.h>

#include <putki/pkgloader.h>
#include <putki/pkgmgr.h>

#include <stdio.h>

// binding up the blob loads.
namespace outki { void bind_test_project_loaders(); }

int main(int argc, char *argv[])
{
	std::cout << "Test-App launching!" << std::endl;
	outki::bind_test_project_loaders();

	outki::pkgmgr::loaded_package *pkg = outki::pkgloader::from_file("out/win32/packages/everything.pkg");
	if (!pkg)
	{
		std::cout << "Failed loading everything pkg!" << std::endl;
	}
	
	std::cout << "Loaded package! " << pkg << std::endl;

	outki::testobj *b = (outki::testobj *)outki::pkgmgr::resolve(pkg, "haspointer");
	std::cout << "Curken is " << b->curken << std::endl;
	std::cout << "Blob is " << b->dasblob << std::endl;
	
	for (unsigned int i=0;i<b->dasblob->bytes_size;i++)
		std::cout << "Byte is " << (int) b->dasblob->bytes[i] << std::endl;

	outki::pkgmgr::release(pkg);
	
	return 0;
}
