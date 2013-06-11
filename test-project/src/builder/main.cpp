#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>

#include <inki/types/core.h>
#include <inki/types/test.h>

// generated.
namespace inki
{
	void bind_test_project();
}

struct blob_handler : public putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::db::data *input, const char *path, putki::db::data *output, int obj_phase)
	{
		putki::type_handler_i *th;
		putki::instance_t obj;
		if (!putki::db::fetch(input, path, &th, &obj))
			return false;

		inki::gurka *b = (inki::gurka*) obj;

		std::string out;
		for (int i=0;i<b->smaskighet.size();i++)
			out.push_back(toupper(b->smaskighet[i]));

		b->smaskighet = out;

		putki::db::insert(output, path, th, b);
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

	putki::build::commit_package(pkg, pconf, "everything.pkg");
}

int main(int argc, char **argv)
{
	inki::bind_test_project();

	putki::builder::set_builder_configurator(&app_register_handlers);

	return run_putki_builder(argc, argv);
}
