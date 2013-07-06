#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>

// generated.
namespace inki
{
	void bind_ccg_ui();
}

//
void register_font_builder(putki::builder::data *builder);
void register_texture_builder(putki::builder::data *builder);
void register_atlas_builder(putki::builder::data *builder);
void register_screen_builder(putki::builder::data *builder);

void ccg_ui_register_handlers(putki::builder::data *builder)
{
	register_font_builder(builder);
	register_texture_builder(builder);
	register_atlas_builder(builder);
	register_screen_builder(builder);
}

