#include <iostream>

#include <outki/types/core.h>
#include <outki/types/test.h>

#include <putki/pkgloader.h>
#include <putki/pkgmgr.h>
#include <putki/liveupdate/liveupdate.h>

#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
#endif

// binding up the blob loads.
namespace outki { void bind_test_project_loaders(); }

int main(int argc, char *argv[])
{
	putki::liveupdate::init();

	std::cout << "Test-App launching!" << std::endl;
	outki::bind_test_project_loaders();

	putki::liveupdate::data *liveupdate = 0;

	putki::pkgmgr::loaded_package *pkg = putki::pkgloader::from_file("out/win32/packages/everything.pkg");
	if (!pkg)
	{
		std::cout << "Failed loading everything pkg!" << std::endl;
	}
	
	std::cout << "Loaded package! " << pkg << std::endl;

	outki::testobj *b = (outki::testobj *)putki::pkgmgr::resolve(pkg, "haspointer");	
	outki::magnus *m = (outki::magnus*)putki::pkgmgr::resolve(pkg, "TEST1");	
	outki::gurka *g = (outki::gurka*)putki::pkgmgr::resolve(pkg, "gurk1");

	while (true)
	{
		
		// 
		LIVE_UPDATE(&b);

		std::cout << "Dasblob: ";
		for (unsigned int i=0;i<b->dasblob->bytes_size;i++)
			std::cout << " " << (int) b->dasblob->bytes[i] << std::endl;
		std::cout << std::endl;
		std::cout << "stora kalasets tårta: " << b->stora_kalaset.vilken_tarta << std::endl;

		LIVE_UPDATE(&m);

		std::cout << "OLLE: " << m->olle <<std::endl;
		std::cout << "PELLE: " << m->pella <<std::endl;


		LIVE_UPDATE(&g);
		std::cout << "smaskighet: " << g->smaskighet <<std::endl;
		std::cout << "tjocklek: " << g->tjocklek <<std::endl;

		//////////////////////////////////////////////////////
		if (!liveupdate)
		{
			liveupdate = putki::liveupdate::connect();
		}
		else 
		{
			if (!putki::liveupdate::connected(liveupdate))
			{
				putki::liveupdate::disconnect(liveupdate);
				liveupdate = 0;
			}
			else
			{
				putki::liveupdate::update(liveupdate);
			}
		}
		
		Sleep(50);
	}

	putki::pkgmgr::release(pkg);

	if (liveupdate)
		putki::liveupdate::disconnect(liveupdate);
	
	return 0;
}

