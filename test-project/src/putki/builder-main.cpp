#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/build-db.h>

#include <inki/types/core.h>
#include <inki/types/test.h>

// generated.
namespace inki
{
	void bind_test_project();
}

struct blob_handler : public putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::gurka *b = (inki::gurka*) obj;

		std::string out;
		for (unsigned int i=0;i<b->smaskighet.size();i++)
			out.push_back(toupper(b->smaskighet[i]));

		b->smaskighet = out;

		putki::db::insert(output, path, inki::gurka::th(), b);
		return true;
		
	}
};

void app_register_handlers(putki::builder::data *builder)
{
	static blob_handler bh;
	add_data_builder(builder, "gurka", 0xff, &bh);
}

void app_build_packages(putki::db::data *out, putki::build::packaging_config *pconf)
{
	putki::package::data *pkg = putki::package::create(out);

	putki::package::add(pkg, "haspointer", true);
	putki::package::add(pkg, "TEST1", true);
	putki::package::add(pkg, "gurk1", true);
	putki::package::add(pkg, "levels/1/bleh", true);

	putki::build::commit_package(pkg, pconf, "everything.pkg");
}

int run_putki_builder(int argc, char **argv);

int main(int argc, char **argv)
{
	inki::bind_test_project();

	putki::builder::set_builder_configurator(&app_register_handlers);
	putki::builder::set_packager(&app_build_packages);

	return run_putki_builder(argc, argv);
}
