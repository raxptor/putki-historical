putki
=====

Putki - Generic data system with C++ and C# support, with Mono/GTK-based editor too.

Define own data types through .typedef files. The Putki compiler transforms these into generated code that performs:

* Loading and saving of .json file through C++ routines
* Data build step, with fully customizable data transforms for all your types
* Automatic packaging packaging from .json to a compact binary optimized form 
* Loading routines to resolve pointers once the binary data is in memory.

The generated code is split into two domains. Inki and Outki. Inki is the .json data. Outki is the binary data as loaded in the runtime. Putki comes with a tiny runtime library which handles loading binary package blobs and resolving pointers for you.
