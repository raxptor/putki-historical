#include <iostream>
#include <putki/blob.h>
#include <outki/types/core.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
	std::cout << "Test-App launching!" << std::endl;

	char tmp[65536];
	FILE *fp = fopen("output/bleh", "rb");
	int num = fread(tmp, 1, 65536, fp);
	fclose(fp);

	if (num > 0)
	{
		outki::difficulties *diff = (outki::difficulties *) tmp;
		char *consume = outki::post_blob_load_difficulties(diff, (char*)(diff+1), tmp + num);
		if (consume == tmp + num)
		{
			//

			std::cout << "Kalle is:[" << diff->kalle << "]" << std::endl;

			for (unsigned int i=0;i!=diff->difficulty_count;i++)
			{
				std::cout << i << "=> " << diff->difficulty[i].name << "/" << diff->difficulty[i].level << std::endl;
			}
		}
		else
		{
			std::cout << "blob read failed" << std::endl;
		}
	}


	return 0;
}
