putki
=====

Putki - Generic data system with C++ and C# support, with Mono/GTK-based editor too.

The system allows for defining custom data types through .typedef files. Here is one example.

Types
-----

> Globalsettings
> {
>	string windowtitle
>	u32 window_width
>	u32 window_height
>	ptr Texture icon
>	ptr ShaderProgram shader_solid
>	ptr ShaderProgram shader_texture
> }

The compiler then compiles this into automatically generated (c++) code for the following:

- Parsing the structure from JSON files
- Writing the structure into binary format for different machine configuratinos
- Loading the structure from the same binary format
- Code for traversing the structure (following pointers)

Builder
-------

When using this system you will typically arrange your data like this

data/obj/<JSON files here>
data/res/<Any other resources>

Then you run the data builder on this, where you specify what assets you want, and what the resulting packages will be. 

> putki::package::data *pkg = putki::package::create(out);
> putki::package::add(pkg, "ui/mainmenu/screen", true);
> putki::build::commit_package(pkg, pconf, "mainmenu.pkg");

This then grabs ui/mainmenu/screen(.json), pulls in all other references objects and writes them into a binary package.

All this happens during the build step (where you can also do any processing you want to transform the data, including adding new output
objects etc).

Runtime
-------

When your application wants to load the data, it loads in the putki runtime library, which is quite tiny and efficient, but can load these packages straight into memory.
The process for loading is to load the file (minus header) into memory, resolve pointers and then everything is ready to go.

> // package load
> putki::pkgmgr::loaded_package *pkg = putki::pkgloader::from_file("mainmenu.pkg");
>
> // grab pointer to the main menu (that was json object)
> outki::ui_screen *menu = putki::pkgmgr::resolve(pkg, "ui/mainmenu/screen");

In the runtime, all the strings are no longer std::strings as in the build step, and the arrays are no std::vectors<>.

From the definition

> Example
> {
>    string txt
>    byte[] data
> }

would then be generated

> struct Example
> {
>    const char *txt
>    unsigned char *data;
>    unsigned int data_size
>

All those data bytes and the strings are pointers into the loaded package file and need no dynamic allocation. When loading a packag it will include all the data already.

Patches
-------

The system also supports making incremental builds and writing patch packages, that reference already existing packages but add anything that was modified and added. 

