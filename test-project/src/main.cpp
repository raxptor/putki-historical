#include <iostream>
#include <putki/blob.h>

#include <putki/types/core.h>
#include <outki/types/core.h>

int main(int argc, char **argv)
{
	putki::difficulty_setting ds0;
	ds0.name = "Enkelt";
	ds0.level = 666;
	putki::difficulty_setting ds1;
	ds1.name = "Svårt";
	ds1.level = 4321;
	
	putki::difficulties ds;
	ds.difficulty.push_back(ds0);
	ds.difficulty.push_back(ds1);
	ds.difficulty.push_back(ds0);

	char buf[256];

	for (int i=0;i<200;i++)	
	{
		std::cout << "Writing into " << i << " bytes." << std::endl;
		char *end = buf + i;
		char *out = putki::write_difficulties_into_blob(&ds, buf, end);
		if (!out)
			continue;
//			std::cout << "Packing failed!" << std::endl;
		else {
	
			std::cout << "Packed into " << (out - buf) << " bytes." << std::endl;
//			for (int j=0;j<32;j++)
//				std::cout << (int)buf[j] << std::endl;

			outki::difficulties *ods = (outki::difficulties *) &buf[0];
			char *auxstart = (char*) &ods[1];

			std::cout << "Packed buffer has " << out - auxstart << " aux bytes" << std::endl;

			char *res = outki::post_blob_load_difficulties(ods, auxstart, out);
			if (res == out)
			{
				std::cout << "Successful unpack." << std::endl;
				std::cout << "Number of difficulties: " << ods->difficulty_count << std::endl;
				for (int i=0;i<ods->difficulty_count;i++)
				{
					outki::difficulty_setting *ds = &ods->difficulty[i];
					std::cout << "[ENTRY " << i << "]" << std::endl;
					std::cout << "name: " << ds->name << std::endl;
					std::cout << "level: " << ds->level << std::endl;
				}
				break;
			}
			else
			{
				if (res == 0)
					std::cout << "Unpack got nulled!" << std::endl;
				else
					std::cout << "Unpack failed, " << (out-res) << " excess bytes!" << std::endl;
			}

		}
	}

	return 0;
}
