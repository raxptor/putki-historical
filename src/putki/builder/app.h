#ifndef __PUTKI_APP_H__
#define __PUTKI_APP_H__

// This is what the application must implement.
void app_register_handlers(putki::builder::data *builder);
void app_build_packages(putki::db::data *out, putki::build::packaging_config *pconf);

#endif