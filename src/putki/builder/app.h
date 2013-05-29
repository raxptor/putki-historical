#ifndef __PUTKI_APP_H__
#define __PUTKI_APP_H__

#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/db.h>

// This is what the application must implement.
void app_register_handlers(putki::builder::data *builder);
void app_build_packages(putki::db::data *out, putki::build::packaging_config *pconf);

// Functios the app can call.
int run_putki_builder(int argc, char **argv);

#endif